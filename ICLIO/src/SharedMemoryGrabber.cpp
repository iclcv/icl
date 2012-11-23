/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/SharedMemoryGrabber.cpp                      **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Viktor Richter                    **
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
#include <ICLIO/ImageCompressor.h>

#include <QtCore/QSharedMemory>
#include <QtCore/QProcess>
#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtCore/QMutexLocker>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace io{
    
    struct SharedMemoryGrabberImpl::Data{

        Data():acquireMutex(QMutex::Recursive){}

        QSharedMemory mem;
        ImgBase *image;
        ImgBase *converted_image;
        bool omitDoubledFrames; // todo implement this feature!
        Time lastImageTimeStamp;
        Time lastValidImageGrabbed;
        ImageCompressor compressor;

        bool isNew(const Time &t, Grabber &g){
          if(t == Time::null){
            WARNING_LOG("SharedMemoryGrabber received image with null-TimeStamp while \"omit-doubled-frames\" feature was activated. Deactivating \"omit-doubled-frames\"-feature to avoid dead-locks!");
            g.setPropertyValue("omit-doubled-frames", false);
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
            g.setPropertyValue("omit-doubled-frames",false);
            return false;
          }else if(t == lastImageTimeStamp){
            return false;
          }else{
            lastValidImageGrabbed = Time::now();
            lastImageTimeStamp = t;
            return true;
          }
        }

        QMutex acquireMutex;
        bool callbacksEnabled;

        struct CallbackCaller : public QThread{
            SharedMemoryGrabberImpl *impl;
            SharedMemoryGrabberImpl::Data *data;
            CallbackCaller(SharedMemoryGrabberImpl *impl, SharedMemoryGrabberImpl::Data *data):
              impl(impl),data(data){
              start();
            }
            virtual void run(){
              while(true){
                while(!data->callbacksEnabled){
                  Thread::sleep(100);
                }
                impl->notifyNewImageAvailable(impl->acquireImage());
              }
            }
        } *caller;



    };
    
    SharedMemoryGrabberImpl::SharedMemoryGrabberImpl(const std::string &sharedMemorySegmentID) throw(ICLException):
      m_data(new Data){
      
      m_data->image = 0;
      m_data->converted_image = 0;
      m_data->omitDoubledFrames = true;
      m_data->callbacksEnabled = false;
      m_data->caller = new Data::CallbackCaller(this,m_data);

      if(sharedMemorySegmentID.length()){
        m_data->mem.setKey(sharedMemorySegmentID.c_str());
        if(!m_data->mem.attach(QSharedMemory::ReadOnly)){
          throw ICLException(str(__FUNCTION__)+": unable to connect to shared memory segment \"" + sharedMemorySegmentID + "\"");
        }
      }

      addProperty("omit-doubled-frames", "flag", "", m_data->omitDoubledFrames, 0, "");
      addProperty("enable-callbacks", "flag", "", m_data->callbacksEnabled, 0, "");

      Configurable::registerCallback(utils::function(this,&SharedMemoryGrabberImpl::processPropertyChange));
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
    
    const ImgBase* SharedMemoryGrabberImpl::acquireImage(){
      QMutexLocker lock(&m_data->acquireMutex);

      if(!m_data->mem.isAttached()) throw ICLException(str(__FUNCTION__)+": grabber is currently not attached to shared memory segment");

      m_data->mem.lock();
      while(m_data->omitDoubledFrames && // WAIT FOR NEW IMAGE LOOP
            !m_data->isNew(m_data->compressor.pickTimeStamp((const icl8u*)m_data->mem.constData()),*this) ){
        m_data->mem.unlock();
        Thread::msleep(1);
        m_data->mem.lock();
      }
      
      m_data->compressor.uncompress((const icl8u*)m_data->mem.constData(), m_data->mem.size(), &m_data->image);
      m_data->mem.unlock();
      return m_data->image;

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

    // callback for changed configurable properties
    void SharedMemoryGrabberImpl::processPropertyChange(const utils::Configurable::Property &prop){
      if(prop.name == "omit-doubled-frames"){
        m_data->omitDoubledFrames = parse<bool>(prop.value);
        if(!m_data->omitDoubledFrames && m_data->callbacksEnabled){
          WARNING_LOG("setting omitDoubledFrames to false will also set callbacksEnabled to false");
          setPropertyValue("enable-callbacks", false);
        }
      } else if(prop.name == "enable-callbacks"){
        if(parse<bool>(prop.value) && !m_data->omitDoubledFrames) {
          WARNING_LOG("enabling enable-callbacks will also enable omitDoubledFrames");
          setPropertyValue("omit-doubled-frames", true);
        } else {
          m_data->callbacksEnabled = parse<bool>(prop.value);
        }
      }
    }

    REGISTER_CONFIGURABLE(SharedMemoryGrabber, return new SharedMemoryGrabber(""));
    
  } // namespace io
}

