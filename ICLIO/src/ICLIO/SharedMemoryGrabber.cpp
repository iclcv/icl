/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/SharedMemoryGrabber.cpp                **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Viktor Richter                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLIO/SharedMemoryGrabber.h>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/Thread.h>
#include <ICLIO/ImageCompressor.h>
#include <ICLIO/SharedMemorySegment.h>

#include <QtCore/QProcess>
#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtCore/QMutexLocker>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace io{
    
    static std::string ICL_IMGBASE_STREAM_PREPEND = "icl.core.imgbase.";

    static void addDevicesToList(std::vector<GrabberDeviceDescription> &deviceList){
      std::set<std::string> set = SharedMemorySegmentRegister::getSegmentSet();
      std::set<std::string>::iterator it;
      for(it = set.begin(); it != set.end(); ++it){
        std::string s = *it;
        if(s.find(ICL_IMGBASE_STREAM_PREPEND) != s.npos){
          std::string name = s.substr(ICL_IMGBASE_STREAM_PREPEND.size());
          deviceList.push_back(GrabberDeviceDescription("sm",name,name));
        }
      }
    }

    struct SharedMemoryGrabber::Data{

        Data():acquireMutex(QMutex::Recursive){}

        SharedMemorySegment mem;
        ImgBase *image;
        ImgBase *converted_image;
        bool omitDoubledFrames; // todo implement this feature!
        Time lastImageTimeStamp;
        Time lastValidImageGrabbed;
        ImageCompressor compressor;
        bool loopWarning;

        bool isNew(const Time &t, Grabber &g){
          if(t == Time::null){
            WARNING_LOG("SharedMemoryGrabber received image with null-TimeStamp while \"omit-doubled-frames\" feature was activated. Deactivating \"omit-doubled-frames\"-feature to avoid dead-locks!");
            g.setPropertyValue("omit-doubled-frames", false);
            lastValidImageGrabbed = Time::now();
            return true;
          }else if(lastImageTimeStamp == Time::null){
            lastImageTimeStamp = t;
            lastValidImageGrabbed = Time::now();
            WARNING_LOG("Init grabber time stamp");
            return true;
          }else if(lastImageTimeStamp > t){
            if(loopWarning == true){
            	WARNING_LOG("SharedMemoryGrabber received an image with an older timestamp than the last one. Loop? This warning is only displayed once.");// Deactivating \"omit-doubled-frames\"-feature to avoid dead-locks!");
            	loopWarning=false;
            }
            lastValidImageGrabbed = Time::now();
            lastImageTimeStamp=t;
            return true;
          //}else if( (Time::now() - lastValidImageGrabbed).toSeconds() > 5){
          }else if( (Time::now() - t).toSeconds() > 5 && (Time::now() - lastValidImageGrabbed).toSeconds() > 5){
            WARNING_LOG("SharedMemoryGrabber already waited 5 seconds for a new image which might be caused by an image source that does not provide usefull timestamps. Therefore the 'omit-doubled-frames'-property is deactivated automatically!");
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
            SharedMemoryGrabber *impl;
            SharedMemoryGrabber::Data *data;
            CallbackCaller(SharedMemoryGrabber *impl, SharedMemoryGrabber::Data *data):
              impl(impl),data(data){
              start();
            }
            virtual void run(){
              while(true){
                while(!data->callbacksEnabled){
                  Thread::msleep(100);
                }
                try{
                const ImgBase* ptr = impl->acquireImage();
                impl->notifyNewImageAvailable(ptr);
                } catch (ICLException &e){
                  DEBUG_LOG("catched " << e.what());
                }
              }
            }
        } *caller;



    };
    
    SharedMemoryGrabber::SharedMemoryGrabber(const std::string &sharedMemorySegmentID) throw(ICLException):
      m_data(new Data){
      
      m_data->image = 0;
      m_data->converted_image = 0;
      m_data->omitDoubledFrames = true;
      m_data->callbacksEnabled = false;
      m_data->caller = new Data::CallbackCaller(this,m_data);
      m_data->loopWarning=true;

      if(sharedMemorySegmentID.length()){
        try{
          m_data->mem.reset(ICL_IMGBASE_STREAM_PREPEND+sharedMemorySegmentID, 0);
        } catch (ICLException &e){
          DEBUG_LOG("ERROR: " << e.what());
          throw ICLException(str(__FUNCTION__)+": unable to connect to shared memory segment \"" + sharedMemorySegmentID + "\"");
        }
      }

      addProperty("format", "info", "RGB", "", 0, "");
      addProperty("size", "info", "", "", 0, "");
      addProperty("omit-doubled-frames", "flag", "", m_data->omitDoubledFrames, 0, "");
      addProperty("enable-callbacks", "flag", "", m_data->callbacksEnabled, 0, "");

      Configurable::registerCallback(utils::function(this,&SharedMemoryGrabber::processPropertyChange));
    }

    void SharedMemoryGrabber::init(const std::string &sharedMemorySegmentID) throw (ICLException){
      try{
        m_data->mem.reset(ICL_IMGBASE_STREAM_PREPEND+sharedMemorySegmentID, 0);
      } catch (ICLException &e){
        DEBUG_LOG("ERROR: " << e.what());
        throw ICLException(str(__FUNCTION__)+": unable to connect to shared memory segment \"" + sharedMemorySegmentID + "\"");
      }
    }
    
    SharedMemoryGrabber::~SharedMemoryGrabber(){
      ICL_DELETE(m_data->image);
      ICL_DELETE(m_data->converted_image);
      delete m_data;
    }

    const std::vector<GrabberDeviceDescription> &SharedMemoryGrabber::getDeviceList(bool rescan){
      static std::vector<GrabberDeviceDescription> deviceList;

      if(rescan){
        deviceList.clear();
        addDevicesToList(deviceList);
      }
      return deviceList;
    }
    
    static inline bool has_params(const ImgBase &i, depth d, const Size &s, format f){
      return (i.getDepth() == d) && (i.getSize() == s) && (i.getFormat() == f);
    }
    
    const ImgBase* SharedMemoryGrabber::acquireImage(){
      QMutexLocker lock(&m_data->acquireMutex);

      if(!m_data->mem.lock()) throw ICLException(str(__FUNCTION__)+": can't get lock for shared memory segment");

      while(m_data->omitDoubledFrames && // WAIT FOR NEW IMAGE LOOP
            !m_data->isNew(m_data->compressor.pickTimeStamp((const icl8u*)m_data->mem.constData()),*this)){
        m_data->mem.unlock();
        Thread::msleep(1);
        m_data->mem.lock();
      }

      if(m_data->mem.isEmpty()){
        m_data->mem.unlock();
        return NULL;
      }

      m_data->compressor.uncompress((const icl8u*)m_data->mem.constData(), m_data->mem.getSize(), &m_data->image);
      m_data->mem.unlock();
      if(Size(getPropertyValue("size")) != m_data->image->getSize()){
        setPropertyValue("size", m_data->image->getSize());
      }

      return (m_data->image->getDim()) ? m_data->image : NULL;
    }

    void SharedMemoryGrabber::resetBus(bool verbose){
      SharedMemorySegmentRegister::resetBus();
    }

    // callback for changed configurable properties
    void SharedMemoryGrabber::processPropertyChange(const utils::Configurable::Property &prop){
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


    Grabber* createSMGrabber(const std::string &param){
      return new SharedMemoryGrabber(param);
    }

    const std::vector<GrabberDeviceDescription>& getSMDeviceList(std::string hint, bool rescan){
      static std::vector<GrabberDeviceDescription> deviceList;
      if(!rescan) return deviceList;

      deviceList.clear();
      addDevicesToList(deviceList);
      // if filter exists, add grabber with filter
      if(hint.size()) deviceList.push_back(
        GrabberDeviceDescription("sm", hint, "A grabber for images published via SharedMemory.")
        );
      return deviceList;
    }

    REGISTER_GRABBER(sm,utils::function(createSMGrabber), utils::function(getSMDeviceList), "sm:shared memory segment name:Qt-based shared memory source");
    REGISTER_GRABBER_BUS_RESET_FUNCTION(sm,SharedMemoryGrabber::resetBus);
    
  } // namespace io
}

