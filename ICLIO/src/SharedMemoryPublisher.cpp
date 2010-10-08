/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/SharedMemoryPublisher.cpp                    **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLIO/SharedMemoryPublisher.h>
#include <QtCore/QSharedMemory>
#include <ICLUtils/StringUtils.h>
#include <ICLCore/ImageSerializer.h>
#include <ICLCore/Img.h>

namespace icl{
  
  struct SharedMemoryLocker{
    QSharedMemory &mem;
    SharedMemoryLocker(QSharedMemory &mem):
      mem(mem){
      mem.lock();
    }
    ~SharedMemoryLocker(){ 
      mem.unlock() ; 
    }
  };
  
  std::vector<std::string> extract_grabber_list(QSharedMemory &mem){
    ICLASSERT_THROW(mem.size() >= (int)sizeof(icl32s),ICLException("unable to extract list of shared memory grabbers (code 1)"));
    const char* data = (const char*)mem.constData();
    std::vector<std::string> gs(*(icl32s*)data);
    data += sizeof(icl32s);
    for(unsigned int i=0;i<gs.size();++i){
      gs[i] = data;
      data += gs[i].length()+1;
    }
    return gs;
  }
  
  void set_grabber_list(QSharedMemory &mem,const std::vector<std::string> &gs){
    char *data = (char*)mem.data();
    *(icl32s*)data = gs.size();
    data += sizeof(icl32s);
    for(unsigned int i=0;i<gs.size();++i){
      std::copy(&gs[i][0],&gs[i][0]+gs[i].length(),data);
      data+= gs[i].length();
      *data++ = '\0';
    }
  }

  static void register_grabber(QSharedMemory &mem,const std::string &s){
    SharedMemoryLocker lock(mem);
    if(!mem.attach(QSharedMemory::ReadWrite)){
      if(!mem.create(100000)){
        ERROR_LOG("unable to create or attach to shared memory segment 'icl-shared-mem-grabbers'");
        return;
      }
    }
    std::vector<std::string> grabbers = extract_grabber_list(mem);
    
    if(find(grabbers.begin(),grabbers.end(),s) != grabbers.end()){
      ERROR_LOG("SharedMemoryGrabber with ID \"" << s << "\" was instantiated twice!");
      return;
    }
    grabbers.push_back(s);

    set_grabber_list(mem,grabbers);
  }
  
  static void unregister_grabber(QSharedMemory &mem,const std::string &s){
    SharedMemoryLocker lock(mem);
    if(!mem.attach(QSharedMemory::ReadWrite)){
      ERROR_LOG("unable remove shared memory grabber instance \"" << s << "\" from device list (no connection to memory segment, code 3))");
      return;
    }
    std::vector<std::string> grabbers = extract_grabber_list(mem);
    
    std::vector<std::string>::iterator it = find(grabbers.begin(),grabbers.end(),s);
    if(it == grabbers.end()){
      ERROR_LOG("SharedMemoryGrabber with ID \"" << s << "\" was not found in the list (code 4)");
      return;
    }
    grabbers.erase(it);

    set_grabber_list(mem,grabbers);
  }
  
  struct SharedMemoryPublisher::Data{
    QSharedMemory mem;
    QSharedMemory listMem;
    std::string name;
  };
  
  SharedMemoryPublisher::SharedMemoryPublisher(const std::string &memorySegmentName) throw (ICLException){
    m_data = new Data;
    m_data->name = memorySegmentName;
    m_data->listMem.setKey("icl-shared-mem-grabbers");
    
    Img8u image;
    publish(&image);
  }
  
  
  SharedMemoryPublisher::~SharedMemoryPublisher(){
    unregister_grabber(m_data->listMem,m_data->name);
    delete m_data;
  }
    

  void SharedMemoryPublisher::createPublisher(const std::string &memorySegmentName) throw (ICLException){
    QSharedMemory &mem = m_data->mem;
    mem.lock();
    if(mem.isAttached()){
      mem.detach();
    }
    m_data->name = memorySegmentName;
    mem.unlock();
    
    Img8u tmp;
    publish(&tmp);
  }
    
  void SharedMemoryPublisher::publish(const ImgBase *image){
    QSharedMemory &mem = m_data->mem;

    int imageSize = ImageSerializer::estimateSerializedSize(image);
    
    if(!mem.isAttached()){
      if(!m_data->name.length()) throw ICLException(str(__FUNCTION__)+": no memory segment name was defined yet!");
      mem.setKey(m_data->name.c_str());
      mem.lock();
      if(m_data->mem.attach(QSharedMemory::ReadWrite)){
        WARNING_LOG(str(__FUNCTION__)+": memory segment did already exist before (using it)!");
        register_grabber(m_data->listMem,m_data->name);
      }else{
        if(mem.create(imageSize)){
          register_grabber(m_data->listMem,m_data->name);
        }
      }
      mem.unlock();
    }
    ICLASSERT_THROW(mem.isAttached(),ICLException(str(__FUNCTION__)+": QSharedMemory is still not attached (this should not happen)" ));


    if(mem.size() != imageSize){
      mem.detach();
      if(!mem.create(imageSize)){
        throw ICLException(str(__FUNCTION__)+": unable to resize memory segment (it seems to be in use from somewhere else)");
      }
    }

    mem.lock();    
    ImageSerializer::serialize(image,(icl8u*)mem.data());
    mem.unlock();
  }

  std::string SharedMemoryPublisher::getMemorySegmentName() const throw (ICLException){
    return m_data->name;
  }
}

