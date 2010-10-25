/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/SharedMemoryGrabber.cpp                      **
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

#include <ICLIO/SharedMemoryGrabber.h>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/Thread.h>
#include <ICLCore/ImageSerializer.h>

#include <QtCore/QSharedMemory>
#include <QtCore/QProcess>

namespace icl{
  
  struct SharedMemoryGrabberImpl::Data{
    QSharedMemory mem;
    ImgBase *image;
    ImgBase *converted_image;
    bool omitDoubledFrames; // todo implement this feature!
    Time lastImageTimeStamp;
    Time lastValidImageGrabbed;
    
    bool isNew(const Time &t, Grabber &g){
      if(t == Time::null){
        WARNING_LOG("SharedMemoryGrabber received image with null-TimeStamp while \"omit-doubled-frames\" feature was activated. Deactivating \"omit-doubled-frames\"-feature to avoid dead-locks!");
        omitDoubledFrames = false;
        lastValidImageGrabbed = Time::now();
        return true;
      }else if(lastImageTimeStamp == Time::null){
        lastImageTimeStamp = t;
        lastValidImageGrabbed = Time::now();
        return true;
      }else if(lastImageTimeStamp > t){
        WARNING_LOG("SharedMemoryGrabber received an image with an older timestamp than the last one. Deactivating \"omit-doubled-frames\"-feature to avoid dead-locks!");
        lastValidImageGrabbed = Time::now();
        return true;
      }else if( (Time::now() - lastValidImageGrabbed).toSeconds() > 5){
        WARNING_LOG("SharedMemoryGrabber alread waited 5 seconds for a new image which might be caused by an image source that does not provide usefull timestamps. Therefore the 'omit-doubled-frames'-property is deactivated automatically!");
        g.setProperty("omit-doubled-frames","off");
        return false;
      }else{
        lastValidImageGrabbed = Time::now();
        lastImageTimeStamp = t;
        return true;
      }
    }
  };
  
  SharedMemoryGrabberImpl::SharedMemoryGrabberImpl(const std::string &sharedMemorySegmentID) throw(ICLException):
    m_data(new Data){
    
    m_data->image = 0;
    m_data->converted_image = 0;
    m_data->omitDoubledFrames = true;
    
    if(sharedMemorySegmentID.length()){
      m_data->mem.setKey(sharedMemorySegmentID.c_str());
      if(!m_data->mem.attach(QSharedMemory::ReadOnly)){
        throw ICLException(str(__FUNCTION__)+": unable to connect to shared memory segment \"" + sharedMemorySegmentID + "\"");
      }
    }
    
    setUseDesiredParams(false);
  }

  void SharedMemoryGrabberImpl::init(const std::string &sharedMemorySegmentID) throw (ICLException){
    if(m_data->mem.isAttached()){
      m_data->mem.lock();
      m_data->mem.detach();
      m_data->mem.setKey(sharedMemorySegmentID.c_str());
      if(!m_data->mem.attach(QSharedMemory::ReadOnly)){
        throw ICLException(str(__FUNCTION__)+": unable to connect to shared memory segment \"" + sharedMemorySegmentID + "\"");
      }
      m_data->mem.unlock();
    }else{
      m_data->mem.lock();
      m_data->mem.setKey(sharedMemorySegmentID.c_str());
      if(!m_data->mem.attach(QSharedMemory::ReadOnly)){
        throw ICLException(str(__FUNCTION__)+": unable to connect to shared memory segment \"" + sharedMemorySegmentID + "\"");
      }
      m_data->mem.unlock();
    }
  }
  
  
  SharedMemoryGrabberImpl::~SharedMemoryGrabberImpl(){
    ICL_DELETE(m_data->image);
    ICL_DELETE(m_data->converted_image);
    delete m_data;
  }
  
  const std::vector<GrabberDeviceDescription> &SharedMemoryGrabberImpl::getDeviceList(bool rescan){
    static std::vector<GrabberDeviceDescription> deviceList;
      
    if(rescan){
      deviceList.clear();
      QSharedMemory mem("icl-shared-mem-grabbers");
      if(!mem.attach(QSharedMemory::ReadOnly)) {
        return deviceList;
      }
      
      mem.lock();
      const char* list = (const char*)mem.constData();
      icl32s num = *(icl32s*)list;
      list +=sizeof(icl32s);
      for(icl32s i=0;i<num;++i){
        deviceList.push_back(GrabberDeviceDescription("sm",list,list));
        list += deviceList.back().id.length()+1;
      }
      mem.unlock();
    }
    return deviceList;
  }
  
  static inline bool has_params(const ImgBase &i, depth d, const Size &s, format f){
    return (i.getDepth() == d) && (i.getSize() == s) && (i.getFormat() == f);
  }
  
  const ImgBase* SharedMemoryGrabberImpl::grabUD(ImgBase **ppoDst){
    if(!m_data->mem.isAttached()) throw ICLException(str(__FUNCTION__)+": grabber is currently not attached to shared memory segment");
    if(getUseDesiredParams()){
      ImgBase **dst = &m_data->image;
      
      m_data->mem.lock();
      while(m_data->omitDoubledFrames && // WAIT FOR NEW IMAGE LOOP
            !m_data->isNew(ImageSerializer::deserializeTimeStamp((const icl8u*)m_data->mem.constData()),*this)){
        m_data->mem.unlock();
        Thread::msleep(1);
        m_data->mem.lock();
      }
      ImageSerializer::deserialize((const icl8u*)m_data->mem.constData(),dst);
      m_data->mem.unlock();
      
      if(has_params(**dst,getDesiredDepth(),getDesiredSize(),getDesiredFormat())){
        return *dst;
      }else{
        if(ppoDst){
          ensureCompatible(ppoDst,getDesiredDepth(),getDesiredSize(),getDesiredFormat());
          m_oConverter.apply(*dst,*ppoDst);
          return *ppoDst;
        }else{
          ensureCompatible(&m_data->converted_image,getDesiredDepth(),getDesiredSize(),getDesiredFormat());
          m_oConverter.apply(*dst,m_data->converted_image);
          return m_data->converted_image;
        }
      }
    }else{
      m_data->mem.lock();
      while(m_data->omitDoubledFrames && // WAIT FOR NEW IMAGE LOOP
            !m_data->isNew(ImageSerializer::deserializeTimeStamp((const icl8u*)m_data->mem.constData()),*this)){

        m_data->mem.unlock();
        Thread::msleep(1);
        m_data->mem.lock();
      }
      // TODO extend deserialize to check for deserialized timestamp first
      ImageSerializer::deserialize((const icl8u*)m_data->mem.constData(),ppoDst ? ppoDst : &m_data->image);
      m_data->mem.unlock();
      Thread::msleep(10);
      
      ImgBase *ret = ppoDst ? *ppoDst : m_data->image;
      SHOW(*ret);
      
      return ppoDst ? *ppoDst : m_data->image;
    }
    return 0;
  }

  void SharedMemoryGrabber::resetBus(){
    QSharedMemory mem("icl-shared-mem-grabbers");
    if(mem.attach(QSharedMemory::ReadWrite)) {
      mem.lock();
      *(icl32s*)mem.data() = 0;
      mem.unlock();
    }else{
      WARNING_LOG("No shared memory segment named 'icl-shared-mem-grabbers' found");
    }

#ifdef SYSTEM_LINUX
    static const std::string QT_SHARED_MEM_PREFIX = "0x51";
  
    QStringList l; l << "-m"; 
    QProcess ipcs;
    ipcs.start("ipcs",l);
    bool ok = ipcs.waitForFinished();
    if(!ok) throw icl::ICLException("unable to call ipcm -m");
    QString stdout = ipcs.readAllStandardOutput();
    
    std::vector<std::string> lines = icl::tok(stdout.toLatin1().data(),"\n");
    for(unsigned int i=0;i<lines.size();++i){
      if(lines[i].substr(0,2) != "0x") continue;
      
      std::vector<std::string> ts = icl::tok(lines[i]," ");
      
      if(ts.size() > 3 && ts[3] == "666" && ts[0].substr(0,4) == QT_SHARED_MEM_PREFIX){
        QProcess ipcrm;
        QStringList l2; l2 << "-m" << ts[1].c_str(); 
        std::cout << "releasing shared memory segment key:" << ts[0] << " shmid:" << ts[1] << std::endl;
        ipcrm.start("ipcrm",l2);
        bool ok = ipcrm.waitForFinished();
        if(!ok) throw icl::ICLException("unable to call ipcrm -m");
      }
    }
#endif

  }


  void SharedMemoryGrabberImpl::setProperty(const std::string &property, const std::string &value){
    if(property == "omit-doubled-frames"){
      if(value == "on") m_data->omitDoubledFrames = true;
      else if(value == "off") m_data->omitDoubledFrames = false;
      else ERROR_LOG("unable to set property 'omit-doubled-frames' to " << value << " (allowed values are 'on' and 'off')");
    }else{
      ERROR_LOG("unable to set unsupported property " << property);
    }
  }
  
  std::vector<std::string> SharedMemoryGrabberImpl::getPropertyList(){
    static const std::string ps[1] = {"omit-doubled-frames"};
    return std::vector<std::string>(ps,ps+1);
  }
  
  std::string SharedMemoryGrabberImpl::getType(const std::string &name){
    if(name == "omit-doubled-frames") {
      return "menu";
    }else{
      ERROR_LOG("invalid property name " << name);
      return "undefined";
    }
  }
  
  std::string SharedMemoryGrabberImpl::getInfo(const std::string &name){
    if(name == "omit-doubled-frames") {
      return "{\"on\",\"off\"}";
    }else{
      ERROR_LOG("invalid property name " << name);
      return "undefined";
    }
  }
  
  std::string SharedMemoryGrabberImpl::getValue(const std::string &name){
    if(name == "omit-doubled-frames") {
      return m_data->omitDoubledFrames ? "true" : "false";
    }else{
      ERROR_LOG("invalid property name " << name);
      return "undefined";
    }
  
  }
  
  
}
  
