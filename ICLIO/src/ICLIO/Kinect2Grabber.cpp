/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/Kinect2Grabber.cpp                     **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Andre Ueckermann                  **
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

#define GLEW_MX
#include <string>
#include <libfreenect2/opengl.h>
#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#undef GLEW_MX

#include <ICLIO/Grabber.h>
#include <ICLIO/Kinect2Grabber.h>
#include <ICLCore/CCFunctions.h>
#include <ICLUtils/StringUtils.h>
#include <ICLCore/Img.h>
#include <ICLCore/CoreFunctions.h>
#include <ICLUtils/Mutex.h>
#include <ICLUtils/Thread.h>
#include <ICLFilter/TranslateOp.h>
#include <ICLUtils/Time.h>
#include <ICLUtils/Lockable.h>



#include <map>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::filter;

namespace icl{
  namespace io{
  
  
    struct LibFreenect2Context : public utils::Lockable{
      libfreenect2::Freenect2 *ctx;
      
      struct Device : public utils::Lockable, public Thread{
        int index;
        libfreenect2::Freenect2Device * dev;
        libfreenect2::SyncMultiFrameListener *listener;
        libfreenect2::FrameMap *frames;
        
        int numUsers;
        
        Img32f irImage, depthImage;
        Img8u rgbImage;
        
        Device(){
          irImage = Img32f(Size(512,424),1);
          depthImage = Img32f(Size(512,424),1);
          rgbImage = Img8u(Size(1920,1080),formatRGB);
        }
        
        virtual void run(){
          while(true){
            Thread::lock();
            listener->waitForNewFrame(*frames);
            
            Time t = Time::now();
            
            libfreenect2::Frame *rgb = (*frames)[libfreenect2::Frame::Color];
            libfreenect2::Frame *ir = (*frames)[libfreenect2::Frame::Ir];
            libfreenect2::Frame *depth = (*frames)[libfreenect2::Frame::Depth];
            
            Mutex::Locker lock(this);
            interleavedToPlanar(rgb->data, &rgbImage);
            rgbImage.swapChannels(0,2);
            
            const float *depthData = reinterpret_cast<const float*>(depth->data);
            const float *irData = reinterpret_cast<const float*>(ir->data);
            std::copy(depthData, depthData+depthImage.getDim(), depthImage.begin(0));
            std::copy(irData, irData+irImage.getDim(), irImage.begin(0));

            listener->release(*frames);
            
            irImage.setTime(t);
            depthImage.setTime(t);
            rgbImage.setTime(t);
            Thread::unlock();
            Thread::msleep(1);
          }
        }
      };
      
      std::set<int> connectedDevices;
      std::map<int,Device*> openDevices;
      
      
      
      LibFreenect2Context(){
        glfwInit();
        ctx = new libfreenect2::Freenect2;
        
        for(int idx=0;idx<8;++idx){
          libfreenect2::Freenect2Device *dev = ctx->openDevice(idx);
          if(dev){
            connectedDevices.insert(idx);
            dev->stop();
            dev->close();
            delete dev;
          }else{
            break;
          }
        }
      }
    public:
      
      Device *openDevice(int idx){
        Mutex::Locker lock(this);
        if(!connectedDevices.count(idx)){
          throw ICLException("cannot open Kinect2 device " + str(idx) + " (device not found!)");
          return 0;
        }
      
        if(openDevices.find(idx) != openDevices.end()){
          openDevices[idx]->numUsers++;
          return openDevices[idx];
        }
      
        Device *dev = new Device;
        dev->numUsers = 1;
        openDevices[idx] = dev;
        dev->index = idx;
        dev->dev = ctx->openDevice(idx);
        if(!dev->dev){
          delete dev;
          throw ICLException("error opening Kinect2 device " + str(idx));
          return 0;
        }
      
        dev->listener = new libfreenect2::SyncMultiFrameListener(libfreenect2::Frame::Color 
                                                                 | libfreenect2::Frame::Ir 
                                                                 | libfreenect2::Frame::Depth); 
        dev->frames = new libfreenect2::FrameMap;
      
        dev->dev->setColorFrameListener(dev->listener);
        dev->dev->setIrAndDepthFrameListener(dev->listener);
        dev->dev->start();
        dev->start(); // grabber thread (grabs ir, depth and rgb in parallel)
      
        return dev;
      }
      
      void freeDevice(Device *dev){
        Mutex::Locker lock(this);
        if(!dev){
          ERROR_LOG("cannot free null-device ??");
          return;
        }
        if(--dev->numUsers == 0){
          dev->stop();
          dev->dev->stop();
          dev->dev->close();
          delete dev->dev;
          delete dev->listener;
          delete dev->frames;
          delete dev;
        }
      }
      

      static LibFreenect2Context &instance(){
        static LibFreenect2Context ctx;
        return ctx;
      }
      
      std::vector<int> getConnectedDeviceList() const{
        Mutex::Locker lock(this);
        return std::vector<int>(connectedDevices.begin(),connectedDevices.end());
      }
    };
    
    
    struct Kinect2Grabber::Impl{
      LibFreenect2Context::Device *dev;
      Kinect2Grabber::Mode mode;
      bool avoidDoubledFrames;
      bool unflipXAxis;
      Img32f irImage, depthImage;
      Img8u rgbImage;

      std::pair<Time,Time> getTimesToCompare() {
        Time a, b;
        switch(mode){
          case Kinect2Grabber::GRAB_RGB_IMAGE:
            a = rgbImage.getTime();
            b = dev->rgbImage.getTime();
            break;
          case Kinect2Grabber::GRAB_DEPTH_IMAGE:
            a = depthImage.getTime();
            b = dev->depthImage.getTime();
            break;
          case Kinect2Grabber::GRAB_IR_IMAGE:
            a = irImage.getTime();
            b = dev->irImage.getTime();
            break;
          default:
            a = Time(77);
            break;
        }
        return std::make_pair(a,b);
      }
      void waitToAvoidDoubledFrames(){
        if(!avoidDoubledFrames) return;
        Time startTime;
        std::pair<Time,Time>  ts = getTimesToCompare();
        while(ts.first == ts.second){
          dev->Lockable::unlock();
          Thread::msleep(1);
          dev->Lockable::lock();
          ts = getTimesToCompare();
          if(startTime.age().toSecondsDouble() > 5){
            DEBUG_LOG("error waiting for double frame avoidance .. deactivating flag");
            avoidDoubledFrames = false;
            return;
          }
        }
      }

      template<class T>
      Img<T> *copyOutput(const Img<T> &src, Img<T> &dst){
        waitToAvoidDoubledFrames();
        if(unflipXAxis){
          flippedCopy(axisVert, &src, bpp(dst));
        }else{
          src.deepCopy(&dst);
        }
        return &dst;
      }

    };
    
    Kinect2Grabber::Kinect2Grabber(Kinect2Grabber::Mode mode, int deviceID) throw (ICLException) {
      if(mode != DUMMY_MODE){
        LibFreenect2Context &ctx = LibFreenect2Context::instance();
        m_impl = new Impl;
        try{
          m_impl->dev = ctx.openDevice(deviceID);
          m_impl->mode = mode;
          m_impl->avoidDoubledFrames = true;
          m_impl->unflipXAxis = true;
        }catch(ICLException &e){
          delete m_impl;
          throw;
        }
      }else{
        m_impl = 0;
      }

      addProperty("Avoid double frames","flag","",1,0,"whether to avoid returning the same frame multiple times");
      addProperty("Unflip X-Axis","flag","",1,1,"The driver's output images are flipped. Decide whether to un-flip it");
      
      switch(mode){
        case GRAB_RGB_IMAGE:
          addProperty("format","menu","RGB-8u","RGB (8u)");
          addProperty("size","menu","FullHD - 1080p 1920x1080","FullHD - 1080p 1920x1080");
          break;
        case GRAB_IR_IMAGE:
          addProperty("format","menu","1 channdel float","1 channel float");
          addProperty("size","menu","512x424","512x424");
          break;
        case GRAB_DEPTH_IMAGE:
          addProperty("format","menu","1 channdel float","1 channel float");
          addProperty("size","menu","512x424","512x424");
          break;
        default:
          break;
      }

      Configurable::registerCallback(utils::function(this,&Kinect2Grabber::processPropertyChange));
    }
    
    Kinect2Grabber::~Kinect2Grabber(){
      if(m_impl){
        LibFreenect2Context &ctx = LibFreenect2Context::instance();
        ctx.freeDevice(m_impl->dev);
        delete m_impl;
      }
    }

    const ImgBase* Kinect2Grabber::acquireImage(){
      Mutex::Locker lock(m_impl->dev);
      switch(m_impl->mode){
        case GRAB_DEPTH_IMAGE:
          //m_impl->waitToAvoidDoubledFrames();
          return m_impl->copyOutput(m_impl->dev->depthImage,m_impl->depthImage);
          //          return &m_impl->depthImage;
          break;
        case GRAB_RGB_IMAGE:
          //m_impl->waitToAvoidDoubledFrames();
          //          m_impl->dev->rgbImage.deepCopy(&m_impl->rgbImage);
          return m_impl->copyOutput(m_impl->dev->rgbImage,m_impl->rgbImage);
          break;
        case GRAB_IR_IMAGE:
          return m_impl->copyOutput(m_impl->dev->irImage,m_impl->irImage);
          //m_impl->waitToAvoidDoubledFrames();
          //          m_impl->dev->irImage.deepCopy(&m_impl->irImage);
           //return &m_impl->irImage;
          break;
        default:
          throw ICLException("Kinect2Grabber::acquireImage() invalid image mode!");
          break;
      }
      return 0;
    }

    /// callback for changed configurable properties
    void Kinect2Grabber::processPropertyChange(const utils::Configurable::Property &prop){
      if(prop.name == "Avoid double frames"){
        m_impl->avoidDoubledFrames = parse<bool>(prop.value);
      }else if(prop.name == "Unflip X-Axis"){
        m_impl->unflipXAxis = parse<bool>(prop.value);
      }
    }

    REGISTER_CONFIGURABLE(Kinect2Grabber, return new Kinect2Grabber(Kinect2Grabber::DUMMY_MODE));

    /// returns a list of attached kinect devices
    const std::vector<GrabberDeviceDescription> &Kinect2Grabber::getDeviceList(bool rescan){
      static std::vector<GrabberDeviceDescription> devices;
      if(rescan){
        std::vector<int> ds = LibFreenect2Context::instance().getConnectedDeviceList();
        devices.clear();
        for(size_t i=0;i<ds.size();++i){
          devices.push_back(GrabberDeviceDescription("kinect2d",str(ds[i]),"Kinect2 Depth Camera (ID "+str(ds[i])+")"));
          devices.push_back(GrabberDeviceDescription("kinect2c",str(ds[i]),"Kinect2 Color Camera RGB (ID "+str(ds[i])+")"));
          devices.push_back(GrabberDeviceDescription("kinect2i",str(ds[i]),"Kinect2 Color Camera IR (ID "+str(ds[i])+")"));
        }
      }
      return devices;
    }



    Grabber* createDepth2Grabber(const std::string &param){
      return new Kinect2Grabber(Kinect2Grabber::GRAB_DEPTH_IMAGE,to32s(param));
    }

    Grabber* createRGB2Grabber(const std::string &param){
      return new Kinect2Grabber(Kinect2Grabber::GRAB_RGB_IMAGE,to32s(param));
    }

    Grabber* createIR2Grabber(const std::string &param){
      return new Kinect2Grabber(Kinect2Grabber::GRAB_IR_IMAGE,to32s(param));
    }

    const std::vector<GrabberDeviceDescription>& getKinect2DDeviceList(std::string hint, bool rescan){
      static std::vector<GrabberDeviceDescription> devices;
      if(rescan){
        std::vector<int> ds = LibFreenect2Context::instance().getConnectedDeviceList();
        devices.clear();
        for(size_t i=0;i<ds.size();++i){
          devices.push_back(GrabberDeviceDescription("kinect2d",str(ds[i]),"Kinect2 Depth Camera (ID "+str(ds[i])+")"));
        }
      }
      return devices;
    }

    const std::vector<GrabberDeviceDescription>& getKinect2CDeviceList(std::string hint, bool rescan){
      static std::vector<GrabberDeviceDescription> devices;
      if(rescan){
        std::vector<int> ds = LibFreenect2Context::instance().getConnectedDeviceList();
        devices.clear();
        for(size_t i=0;i<ds.size();++i){
          devices.push_back(GrabberDeviceDescription("kinect2c",str(ds[i]),"Kinect2 Color Camera (ID "+str(ds[i])+")"));
        }
      }
      return devices;
    }

    const std::vector<GrabberDeviceDescription>& getKinect2IDeviceList(std::string hint, bool rescan){
      static std::vector<GrabberDeviceDescription> devices;
      if(rescan){
        std::vector<int> ds = LibFreenect2Context::instance().getConnectedDeviceList();
        devices.clear();
        for(size_t i=0;i<ds.size();++i){
          devices.push_back(GrabberDeviceDescription("kinect2i",str(ds[i]),"Kinect2 IR Camera (ID "+str(ds[i])+")"));
        }
      }
      return devices;
    }

    REGISTER_GRABBER(kinect2d,utils::function(createDepth2Grabber), utils::function(getKinect2DDeviceList), "kinect2d:device ID:kinect2 depth camera source:");
    REGISTER_GRABBER(kinect2c,utils::function(createRGB2Grabber), utils::function(getKinect2CDeviceList),"kinect2c:device ID:kinect2 color camera source");
    REGISTER_GRABBER(kinect2i,utils::function(createIR2Grabber), utils::function(getKinect2IDeviceList),"kinect2i:devide ID:kinect2 IR camera source");

  } // namespace io
} // namespace icl

