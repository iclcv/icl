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
#include <QtCore/QSharedMemory>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/Thread.h>
#include <ICLCore/ImageSerializer.h>

namespace icl{
  
  struct SharedMemoryGrabberImpl::Data{
    QSharedMemory mem;
    ImgBase *image;
    ImgBase *converted_image;
    bool omitDoubledFrames; // todo implement this feature!
    Time lastImageTimeStamp;
    
    bool isNew(const Time &t){
      if(t == Time::null){
        WARNING_LOG("SharedMemoryGrabber received image with null-TimeStamp while \"omit-doubled-frames\" feature was activated. Deactivating \"omit-doubled-frames\"-feature to avoid dead-locks!");
        omitDoubledFrames = false;
        return true;
      }else if(lastImageTimeStamp == Time::null){
        lastImageTimeStamp = t;
        return true;
      }else if(lastImageTimeStamp > t){
        WARNING_LOG("SharedMemoryGrabber received an image with an older timestamp than the last one. Deactivating \"omit-doubled-frames\"-feature to avoid dead-locks!");
        return true;
      }else if(lastImageTimeStamp == t){
        return false;
      }else{
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
            !m_data->isNew(ImageSerializer::deserializeTimeStamp((const icl8u*)m_data->mem.constData()))){
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
            !m_data->isNew(ImageSerializer::deserializeTimeStamp((const icl8u*)m_data->mem.constData()))){
        m_data->mem.unlock();
        Thread::msleep(1);
        m_data->mem.lock();
      }
      // TODO extend deserialize to check for deserialized timestamp first
      ImageSerializer::deserialize((const icl8u*)m_data->mem.constData(),ppoDst ? ppoDst : &m_data->image);
      m_data->mem.unlock();
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
      WARNING_LOG("Please note: only the shared-memory device list has been cleaned.\n"
                  "Lost memory segments can currently not be accessed / released.");
    }else{
      WARNING_LOG("No shared memory segment named 'icl-shared-mem-grabbers' found");
    }
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
  
