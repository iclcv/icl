/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/KinectGrabber.cpp                      **
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

#include <ICLIO/Grabber.h>
#include <ICLIO/KinectGrabber.h>
#include <ICLCore/CCFunctions.h>
#include <ICLUtils/StringUtils.h>
#include <ICLCore/Img.h>
#include <ICLUtils/Mutex.h>
#include <ICLUtils/Thread.h>
#include <ICLFilter/TranslateOp.h>
#include <ICLFilter/MedianOp.h>
#include <ICLUtils/Time.h>

#include <libfreenect.h>
#include <map>

//#define FREENECT_DEBUG_LOGGING

using namespace icl::utils;
using namespace icl::core;
using namespace icl::filter;

namespace icl{
  namespace io{

    static std::ostream &operator<<(std::ostream &s, const KinectGrabber::Mode &m){
      static std::string names[] = { "GRAB_RGB_IMAGE", "GRAB_BAYER_IMAGE", "GRAB_DEPTH_IMAGE",
                                     "GRAB_IR_IMAGE_8BIT", "GRAB_IR_IMAGE_10BIT"};
      if((int)m >= 0 && (int)m < 5) return s << names[(int)m];
      else return s << "UNDEFINED MODE";
    }

    class FreenectContext : public Thread{

    private:
      int started;
      int errors;
      Mutex startMutex;
      Mutex *ctx_mutex;
      freenect_context *ctx_ptr;
      static const int MAX_ERRORS = 100;
      
    public:
      std::vector<std::string> deviceAssociation;
    private:
      FreenectContext() : started(0), errors(0) {
        static Mutex cMutex;
        static freenect_context *ctx = NULL;
        Mutex::Locker l(cMutex);
        if(!ctx && freenect_init(&ctx, NULL) < 0){
          throw ICLException("unable to create freenect_context");
        }
#ifdef FREENECT_DEBUG_LOGGING
        freenect_set_log_level(ctx, FREENECT_LOG_DEBUG);
#endif
        ctx_mutex = &cMutex;
        ctx_ptr = ctx;

        freenect_device_attributes *attribs  = 0;
        int n = freenect_list_device_attributes(ctx,  &attribs);
        if(n < 1){
          throw ICLException("no Kinect devices found");
        }
          
        for(freenect_device_attributes *a = attribs; a ; a=a->next){
          if(a->camera_serial){
            deviceAssociation.push_back(a->camera_serial);
          }else{
            DEBUG_LOG("Kinect Device camera serial was null");
            deviceAssociation.push_back("null??");
          }
        }
          
        freenect_free_device_attributes(attribs);
      }

    public:

      static FreenectContext &getFreenectContext(){
        static FreenectContext context;
        return context;
      }

      int processEvents(){
        Mutex::Locker l(ctx_mutex);
        //DEBUG_LOG("calling freenect_process_events");
        int x = freenect_process_events(ctx_ptr);
        //DEBUG_LOG("calling freenect_process_events done");
        return x;
      }

      int openDevice(freenect_device **dev, int index){
        Mutex::Locker l(ctx_mutex);
        return freenect_open_device(ctx_ptr, dev, index);
      }

      int numDevices(){
        Mutex::Locker l(ctx_mutex);
        return freenect_num_devices(ctx_ptr);
      }

      void start(){
        Mutex::Locker l(startMutex);
        if(!started++){
          Thread::start();
        }
      }

      void stop(){
        Mutex::Locker l(startMutex);
        if(!--started){
          Thread::stop();
        }
      }

      virtual void run(){
        usleep(1000);
        while(true){
          //DEBUG_LOG("kinect grabber running");
          usleep(1);
          if(!trylock()){
            //DEBUG_LOG("kinect grabber got lock");
            if(processEvents() < 0){
              errors++;
            }
            if(errors > MAX_ERRORS){
              unlock();
              throw ICLException("detected 100th error in freenect event processing");
            }
            unlock();
            //DEBUG_LOG("kinect grabber unlocked");
          }
        }
      }

    };

    /// just copied form libfreenect.hpp
    class FreenectTiltStateICL {
      friend struct FreenectDevice;
      FreenectTiltStateICL(freenect_raw_tilt_state *_state):
        m_code(_state->tilt_status), m_state(_state)
      {}
    public:
      void getAccelerometers(double* x, double* y, double* z) {
        freenect_get_mks_accel(m_state, x, y, z);
      }
      double getTiltDegs() {
        return freenect_get_tilt_degs(m_state);
      }
    public:
      freenect_tilt_status_code m_code;
    private:
      freenect_raw_tilt_state *m_state;
    };

    struct FreenectDevice{
      struct Used{
        enum IRShift{ Off=0,Fast,Accurate } irShift;
        bool depthImageUnitMM;
        int depthImagePostProcessingMedianRadius;
        MedianOp postProcessor3x3;
        MedianOp postProcessor5x5;

        freenect_device *device;
        int numColorUsers;
        int numDepthUsers;
        Mutex colorMutex,depthMutex;
        Img8u colorImage,colorImageOut;
        Img16s irImage16s,irImage16sOut;
        Img8u irImage,irImageOut;
        Img32f depthImage,depthImageOut;
        Time lastColorTime, lastDepthTime;
        Size size;
        std::string serial;

        Used(const std::string &serial):postProcessor3x3(Size(3,3)),postProcessor5x5(Size(5,5)),serial(serial){
          postProcessor3x3.setClipToROI(false);
          postProcessor5x5.setClipToROI(false);
        }

        KinectGrabber::Mode currentColorMode; // this must not be reset as long as the device is used somewhere else


        void depth_cb(void *data, uint32_t timestamp){
          Mutex::Locker lock(depthMutex);
          const int r = depthImagePostProcessingMedianRadius;
          MedianOp *pp = (r == 3) ? &postProcessor3x3 : r == 5 ? &postProcessor5x5 : (MedianOp*)0;

#if 0
          if(serial == "B00365713743108B"){
            // ok this cam needs some manual compensation of the depth values
            static bool first = true;
            if(first){
              first = false;
               DEBUG_LOG("detected Kinect Model " << serial << " adapting raw depth values by factor 1.02");
            }
            icl16s *p = reinterpret_cast<icl16s*>(data);
            const icl16s * const e = p + 640*480;
            for(;p < e;++p){
              if(*p != 2047){
                *p = icl16s(1.02f * *p);
                if(*p > 2047) *p = 2047;
              }
            }
          }
#endif
          if(pp){
            const Img16s tmp(Size::VGA, 1, std::vector<icl16s*>(1, (icl16s*)data));
            pp->apply(&tmp)->convert(&depthImage);
            int b = (r-1)/2;
            depthImage.setROI(Rect(b,b,640-2*b, 480-2*b));
            depthImage.fillBorder(&tmp);
            depthImage.setTime(Time::now());
          }else{
            depthImage.setTime(Time::now());
            std::copy((const icl16s*)data,(const icl16s*)data+640*480,depthImage.begin(0));
          }
        }
        void color_cb(void *data, uint32_t timestamp){
          Mutex::Locker lock(colorMutex);
          switch(currentColorMode){
            case KinectGrabber::GRAB_RGB_IMAGE:
              colorImage.setTime(Time::now());
              interleavedToPlanar((const icl8u*)data,&colorImage);
              break;
            case KinectGrabber::GRAB_IR_IMAGE_8BIT:{
              irImage.setTime(Time::now());
              std::copy((const icl8u*)data,(const icl8u*)data +  (size==Size::VGA ? 640*480 : 320*240), irImage.begin(0));
              break;}
            case KinectGrabber::GRAB_IR_IMAGE_10BIT:
              irImage16s.setTime(Time::now());
              std::copy((const icl16s*)data,(const icl16s*)data +  (size==Size::VGA ? 640*480 : 320*240), irImage16s.begin(0));
              break;
            default:
              // actually, this happens sometimes, when the grabbers 'format' is switched frequently
              // at runtime. This is why, we avoid to throw an exception here!
              // throw ICLException("processed color callback for depth grabber (this should not happen)");
              break;
          }

        }

        static inline float kinect_raw_to_mm(icl16s d){
          return 1.046 * (d==2047 ? 0 : 1000. / (d * -0.0030711016 + 3.3309495161));
        }

        const Img32f &getLastDepthImage(bool avoidDoubleFrames){
          Mutex::Locker lock(depthMutex);
          if(avoidDoubleFrames){
            while(lastDepthTime == depthImage.getTime()){
              depthMutex.unlock();
              Thread::msleep(1);
              depthMutex.lock();
            }
          }
          lastDepthTime = depthImage.getTime();


          if(!depthImageUnitMM){
            depthImage.deepCopy(&depthImageOut);
          }else{
            depthImageOut.setSize(depthImage.getSize());
            depthImageOut.setChannels(1);
            std::transform( depthImage.begin(0), depthImage.end(0), depthImageOut.begin(0), kinect_raw_to_mm);
            depthImageOut.setTime(depthImage.getTime());
          }
          return depthImageOut;
        }
        const ImgBase &getLastColorImage(bool avoidDoubleFrames){
          Mutex::Locker lock(colorMutex);
          ImgBase &src = ( currentColorMode == KinectGrabber::GRAB_RGB_IMAGE ? (ImgBase&)colorImage :
                           currentColorMode == KinectGrabber::GRAB_IR_IMAGE_8BIT ? (ImgBase&)irImage :
                           (ImgBase&)irImage16s);
          ImgBase &dst = ( currentColorMode == KinectGrabber::GRAB_RGB_IMAGE ? (ImgBase&)colorImageOut :
                           currentColorMode == KinectGrabber::GRAB_IR_IMAGE_8BIT ? (ImgBase&)irImageOut :
                           (ImgBase&)irImage16sOut);

          if(avoidDoubleFrames){
            while(lastColorTime == src.getTime()){
              //            DEBUG_LOG("here!");
              colorMutex.unlock();
              Thread::msleep(1);
              colorMutex.lock();
            }
          }

          lastColorTime = src.getTime();

          ImgBase *pDst = &dst;

          if((currentColorMode != KinectGrabber::GRAB_RGB_IMAGE) && (irShift != Off)){
            TranslateOp t(-4.8, -3.9, irShift == Fast ? interpolateNN : interpolateLIN);
            t.apply(&src, &pDst);
            // apply affine warp
          }else{
            src.deepCopy(&pDst);
          }
          return dst;
        }
        void setTiltDegrees(double angle) {
          if(freenect_set_tilt_degs(device, angle) < 0){
            throw ICLException("Cannot set angle in degrees");
          }
        }
        void setLed(freenect_led_options option) {
          if(freenect_set_led(device, option) < 0){
            throw ICLException("Cannot set led");
          }
        }
        void updateState() {
          if (freenect_update_tilt_state(device) < 0){
            throw ICLException("Cannot update device state");
          }
        }
        FreenectTiltStateICL getState() const {
          return FreenectTiltStateICL(freenect_get_tilt_state(device));
        }
      };

      static std::map<int,Used*> devices;
      Used *used;
      KinectGrabber::Mode mode;
      int index;

      void setMode(KinectGrabber::Mode mode, Used *used, const Size &size){
        if(mode != KinectGrabber::GRAB_DEPTH_IMAGE){
          if(used->currentColorMode != KinectGrabber::GRAB_DEPTH_IMAGE
             && used->currentColorMode != mode){
            WARNING_LOG("the color camera mode was changed even though another device with a different mode does still exist");
            used->currentColorMode = mode;
          }else if(used->currentColorMode == KinectGrabber::GRAB_DEPTH_IMAGE){
            used->currentColorMode = mode;
          }
        }

#ifndef FREENECTAPI
        // note, this is a dirty hack, however, it seems to work
        // in the new libfreenect interface, FREENECTAPI is defined to nothing
        // for unix/linux system. Therefore we use this hack here, to
        // distinguish between the new and the old API
        freenect_device *device = used->device;
        const bool isVideo = (mode != KinectGrabber::GRAB_DEPTH_IMAGE);
        static const freenect_video_format fvf[5] = { FREENECT_VIDEO_RGB,
                                                      FREENECT_VIDEO_BAYER,
                                                      (freenect_video_format)-1, // this is used for depth 
                                                      FREENECT_VIDEO_IR_8BIT,
                                                      FREENECT_VIDEO_IR_10BIT };
        if( size != Size::QVGA){
          static bool first = true;
          if(first){
            first = false;
            WARNING_LOG("QVGA size is not supported by the used version of libfreenect (using VGA instead)");
          }
        }
        if(isVideo){
          if( freenect_set_video_format(device, fvf[(int)mode]) < 0)  throw ICLException("Cannot set video format");
        }else{
          if( freenect_set_depth_format(device, FREENECT_DEPTH_11BIT) < 0) throw ICLException("Cannot set depth format");
        }

        used->size = Size::VGA;

#else
        freenect_device *device = used->device;
        freenect_frame_mode m;
        const bool isVideo = (mode != KinectGrabber::GRAB_DEPTH_IMAGE);
        static const freenect_video_format fvf[5] = { FREENECT_VIDEO_RGB,
                                                      FREENECT_VIDEO_BAYER,
                                                      FREENECT_VIDEO_DUMMY,
                                                      FREENECT_VIDEO_IR_8BIT,
                                                      FREENECT_VIDEO_IR_10BIT };
        freenect_resolution res = ( size == Size::VGA ?
                                    FREENECT_RESOLUTION_MEDIUM :
                                    FREENECT_RESOLUTION_LOW);
        if(isVideo){
          m = freenect_find_video_mode(res, fvf[(int)mode]);
        }else{
          m = freenect_find_depth_mode(res, FREENECT_DEPTH_11BIT);
        }

        if (!m.is_valid) throw ICLException("Cannot set video/depth format: invalid mode");

        if(isVideo){
          if(freenect_set_video_mode(device, m) < 0) throw ICLException("Cannot set video format");
        }else{
          if(freenect_set_depth_mode(device, m) < 0) throw ICLException("Cannot set depth format");
        }
        used->size = size;
#endif
      }

      FreenectDevice(int index, KinectGrabber::Mode mode, Size size):mode(mode),index(index){
        FreenectContext &ctx = FreenectContext::getFreenectContext();
        std::map<int,Used*>::iterator it = devices.find(index);
        if(it == devices.end()){
          DEBUG_LOG2("device " << index << " was not used before: creating new one. Mode " << mode);
          used = devices[index] = new Used(ctx.deviceAssociation[index]);
          used->irShift = Used::Accurate;
          used->depthImageUnitMM = true;
          used->currentColorMode = mode;
          used->depthImagePostProcessingMedianRadius = 0;

          if(FreenectContext::getFreenectContext().openDevice(&used->device, index) < 0){
            devices.erase(index);
            throw ICLException("FreenectDevice:: unable to open kinect device for device " + str(index));
          }
          used->numColorUsers = used->numDepthUsers = 0;
          freenect_set_user(used->device, used);
          if(mode != KinectGrabber::GRAB_DEPTH_IMAGE){
            used->numColorUsers++;

            setMode(mode, used, size);

            if(mode == KinectGrabber::GRAB_RGB_IMAGE){
              used->colorImage = Img8u(size,formatRGB);
            }else if(mode == KinectGrabber::GRAB_IR_IMAGE_8BIT){
              used->irImage = Img8u(size,1);
            }else if(mode == KinectGrabber::GRAB_IR_IMAGE_10BIT){
              used->irImage16s = Img16s(size,1);
            }else{
              throw ICLException("FreenectDevice:: invalid color mode detected");
            }

            freenect_set_video_callback(used->device,freenect_video_callback);
            if(freenect_start_video(used->device) < 0){
              throw ICLException("FreenectDevice:: unable to start video for device" + str(index));
            }
          }else{
            DEBUG_LOG2("device " << index << " was used before");
            used->numDepthUsers++;

            setMode(mode, used, size);

            used->depthImage = Img32f(size,formatMatrix);
            freenect_set_depth_callback(used->device,freenect_depth_callback);
            if(freenect_start_depth(used->device) < 0){
              throw ICLException("FreenectDevice:: unable to start depth for device" + str(index));
            }
          }
        }else{ // reuse old device
          //            DEBUG_LOG("device " << index << " was used before: using old one. Type" << mode);
          used = it->second;

          if(used->size != size){
            size = used->size;
            WARNING_LOG("unable to switch kinect device property \"size\":"
                        << " another device with another size is already instantiated");
          }

          if(mode != KinectGrabber::GRAB_DEPTH_IMAGE){
            if(!used->numColorUsers){

              freenect_stop_video(used->device);
              setMode(mode, used, size);

              if(mode == KinectGrabber::GRAB_RGB_IMAGE){
                used->colorImage = Img8u(size,formatRGB);
              }else if(mode == KinectGrabber::GRAB_IR_IMAGE_8BIT){
                used->irImage = Img8u(size,1);
              }else if(mode == KinectGrabber::GRAB_IR_IMAGE_10BIT){
                used->irImage16s = Img16s(size,1);
              }else{
                throw ICLException("FreenectDevice:: invalid color mode detected");
              }

              freenect_set_video_callback(used->device,freenect_video_callback);
              if(freenect_start_video(used->device) < 0){
                throw ICLException("FreenectDevice:: unable to start video for device" + str(index));
              }
            }else{
              if(used->currentColorMode != mode){
                WARNING_LOG("the mode cannot be changed to " << mode
                            << " because another grabber instance with mode "
                            << used->currentColorMode << " does already exist");
              }
            }
            used->numColorUsers++;
          }else{
            if(!used->numDepthUsers){

              freenect_stop_depth(used->device);

              setMode(mode, used, size);

              used->depthImage = Img32f(size,formatMatrix);
              freenect_set_depth_callback(used->device,freenect_depth_callback);
              if(freenect_start_depth(used->device) < 0){
                throw ICLException("FreenectDevice:: unable to start depth for device" + str(index));
              }
            }
          }
        }
      }
      ~FreenectDevice(){
        if(mode != KinectGrabber::GRAB_DEPTH_IMAGE){
          used->numColorUsers--;
          if(!used->numColorUsers){
            used->currentColorMode = KinectGrabber::GRAB_DEPTH_IMAGE;
            if(freenect_stop_video(used->device) < 0){
              throw ICLException("FreenectDevice:: unable to stop color for device "+ str(index));
            }
          }
        }else{
          used->numDepthUsers--;
          if(!used->numDepthUsers){
            if(freenect_stop_depth(used->device) < 0){
              throw ICLException("FreenectDevice:: unable to stop depth for device "+ str(index));
            }
          }
        }
        if(!used->numColorUsers && !used->numDepthUsers){
          if(freenect_close_device(used->device) < 0){
            throw ICLException("FreenectDevice:: unable to close device "+ str(index));
          }
          devices.erase(devices.find(index));
          delete used;
        }
      }

      static void freenect_depth_callback(freenect_device *dev, void *depth, uint32_t timestamp) {
        //SHOW(freenect_get_user(dev));
        static_cast<FreenectDevice::Used*>(freenect_get_user(dev))->depth_cb(depth,timestamp);
      }
      static void freenect_video_callback(freenect_device *dev, void *video, uint32_t timestamp) {
        //SHOW(freenect_get_user(dev));
        static_cast<FreenectDevice::Used*>(freenect_get_user(dev))->color_cb(video,timestamp);
      }
    };

    std::map<int,FreenectDevice::Used*> FreenectDevice::devices;
    
    struct KinectGrabber::Impl{
    private:
      SmartPtr<FreenectDevice> device;

    public:
      int ledColor;
      float desiredTiltDegrees;
      bool avoidDoubleFrames;
      Mutex mutex;
      Time lastupdate;
      //std::string deviceSerialString;
      
      static int getIndex(std::string idOrSerial){
        std::vector<std::string> ts = tok(idOrSerial,"|||",false);
        if(ts.size() > 1){
          idOrSerial = ts[0];
        }
        FreenectContext &ctx = FreenectContext::getFreenectContext();
        if(idOrSerial.length() < 2){
          return parse<int>(idOrSerial);
        }else{
          for(size_t i=0;i<ctx.deviceAssociation.size();++i){
            if(ctx.deviceAssociation[i] == idOrSerial){
              return (int)i;
            }
          }
          throw ICLException("Kinect device serial '" + idOrSerial + "' not found\n"
                             "(found device serials were " + cat(ctx.deviceAssociation,",") + ")");
        }
        return 0;
      }

      Impl(KinectGrabber::Mode mode, std::string idOrSerial, const Size &size)
        : device(new FreenectDevice(getIndex(idOrSerial),mode, size)),
          ledColor(0), desiredTiltDegrees(0), avoidDoubleFrames(true), mutex(Mutex::mutexTypeRecursive), lastupdate(Time::now())
      {
        
          
      }

      ~Impl(){
        device.setNull();
      }

      void switchMode(KinectGrabber::Mode mode, const Size &size){
        if(device->mode != mode || device->used->size != size){
          int idx = device->index;
          //device = SmartPtr<FreenectDevice>();
          device.setNull();
          device = SmartPtr<FreenectDevice>(new FreenectDevice(idx,mode,size));
        }
      }

      inline const Img32f &getLastDepthImage(){
        return device->used->getLastDepthImage(avoidDoubleFrames);
      }
      inline const ImgBase &getLastColorImage(){
        return device->used->getLastColorImage(avoidDoubleFrames);
      }

      SmartPtr<FreenectDevice> &getDevice(){
        return device;
      }
    };

    KinectGrabber::KinectGrabber(KinectGrabber::Mode mode, int deviceID, const Size &size) 
      throw (ICLException) {
      init(mode, str(deviceID),size);
    }
    KinectGrabber::KinectGrabber(KinectGrabber::Mode mode, const std::string &idOrSerial, const Size &size) 
      throw (ICLException) {
      init(mode, idOrSerial,size);
    }
   
    void KinectGrabber::init(KinectGrabber::Mode mode, const std::string &idOrSerial, const Size &size) 
      throw (ICLException){
      
      FreenectContext::getFreenectContext().start();
      m_impl = new Impl(mode,idOrSerial,size);
      // Configurable
      static const std::string formats[] = {
        "Color Image {24Bit RGB}",
        "Bayer Image {8Bit}",
        "Depth Image {float}",
        "IR Image {8Bit}",
        "IR Image {10Bit}"
      };

      static const std::string values[] = {
        "off",
        "fast",
        "accurate"
      };

      try{m_impl->getDevice()->used->updateState();}catch(...){}
      double degs = m_impl->getDevice()->used->getState().getTiltDegs();
      std::string angleval = (degs == -64) ? "moving" : str(degs);
      double a[3]={0,0,0};
      m_impl->getDevice()->used->getState().getAccelerometers(a,a+1,a+2);
      std::string accelval = str(a[0]) + "-" + str(a[1]) + "-" + str(a[2]);
      std::string diunit = (m_impl->getDevice()->used->depthImageUnitMM) ? "mm" : "raw";
      const int r  = m_impl->getDevice()->used->depthImagePostProcessingMedianRadius;
      std::string ppvalue = (r == 3) ? "median 3x3" : ((r == 5) ? "median 5x4" : "off");

      addProperty("Avoid double frames","flag","",m_impl->avoidDoubleFrames,0,"whether to avoid returning the same frame multiple times");
      addProperty("format", "menu", "Color Image {24Bit RGB},Depth Image {float},IR Image {8Bit},IR Image {10Bit}", formats[m_impl->getDevice()->mode], 0, "");
      addProperty("size", "menu", "VGA {640x480}", "VGA {640x480}", 0, "");
      addProperty("LED", "menu", "off,green,red,yellow,blink yellow,blink green,blink red/yellow", m_impl->ledColor, 0, "");
      addProperty("Desired-Tilt-Angle", "range", "[-35,25]", m_impl->desiredTiltDegrees, 0, "");
      addProperty("Current-Tilt-Angle", "info", "", angleval, 100, "");
      addProperty("Accelerometers", "info", "", accelval, 100, "");
      addProperty("shift-IR-image", "menu", "off,fast,accurate", values[(int)(m_impl->getDevice()->used->irShift)], 0, "");
      addProperty("depth-image-unit", "menu", "raw,mm", diunit, 0, "");
      addProperty("depth-image-post-processing", "menu", "off,median 3x3,median 5x5", ppvalue, 0, "");

      Configurable::registerCallback(utils::function(this,&KinectGrabber::processPropertyChange));
    }
    
    KinectGrabber::~KinectGrabber(){
      ICL_DELETE(m_impl);
      FreenectContext::getFreenectContext().stop();
    }

    const ImgBase* KinectGrabber::acquireImage(){
      Mutex::Locker lock(m_impl->mutex);
      // update current angle and accelometers every 200ms
      if(m_impl -> lastupdate.age() > 200000){
        updateState();
      }
      if(m_impl->getDevice()->mode != GRAB_DEPTH_IMAGE){
        return &m_impl->getLastColorImage();
      }else{
        return &m_impl->getLastDepthImage();
      }
    }

    void KinectGrabber::updateState(){
      try{m_impl->getDevice()->used->updateState();}catch(...){DEBUG_LOG("could not update")}
      double degs = m_impl->getDevice()->used->getState().getTiltDegs();
      std::string angleval = (degs == -64) ? "moving" : str(degs);
      double a[3]={0,0,0};
      m_impl->getDevice()->used->getState().getAccelerometers(a,a+1,a+2);
      std::string accelval = str(a[0]) + "-" + str(a[1]) + "-" + str(a[2]);
      prop("Current-Tilt-Angle").value = angleval;
      prop("Accelerometers").value = accelval;
      m_impl -> lastupdate = Time::now();
    }

    

    /// callback for changed configurable properties
    void KinectGrabber::processPropertyChange(const utils::Configurable::Property &prop){
      Mutex::Locker lock(m_impl->mutex);

      if(prop.name == "Avoid double frames"){
        m_impl->avoidDoubleFrames = parse<bool>(prop.value);
        return;
      }

      if(prop.name == "format"){
        static const std::string formats[] = {
          "Color Image {24Bit RGB}",
          "Bayer Image {8Bit}",
          "Depth Image {float}",
          "IR Image {8Bit}",
          "IR Image {10Bit}"
        };
        int idx = (int)(std::find(formats, formats+5, prop.value) - formats);
        if(idx == 5){
          ERROR_LOG("invalid property value for property 'format'");
          return;
        }
        m_impl->switchMode((Mode)idx, m_impl->getDevice()->used->size);

      } else if(prop.name == "size"){
        /*
            if(value != "VGA {640x480}" && value != "QVGA {320x240}"){
            ERROR_LOG("invalid property value for property 'size'");
            }else{
            m_impl->switchMode(m_impl->device->mode,value == "VGA {640x480}" ?
            Size::VGA : Size::QVGA);
            }
            */
      }else if(prop.name == "LED"){
        if(prop.value == "off"){
          m_impl->getDevice()->used->setLed((freenect_led_options)0);
        }else if(prop.value == "green"){
          m_impl->getDevice()->used->setLed((freenect_led_options)1);
        }else if(prop.value == "red"){
          m_impl->getDevice()->used->setLed((freenect_led_options)2);
        }else if(prop.value == "yellow"){
          m_impl->getDevice()->used->setLed((freenect_led_options)3);
        }else if(prop.value == "blink yellow"){
          m_impl->getDevice()->used->setLed((freenect_led_options)4);
        }else if(prop.value == "blink green"){
          m_impl->getDevice()->used->setLed((freenect_led_options)5);
        }else if(prop.value == "blink red/yellow"){
          m_impl->getDevice()->used->setLed((freenect_led_options)6);
        }else{
          ERROR_LOG("invalid property value for property 'LED'" << prop.value);
        }
      }else if(prop.name == "Desired-Tilt-Angle"){
        m_impl->getDevice()->used->setTiltDegrees(parse<double>(prop.value));
      }else if(prop.name == "shift-IR-image"){
        if(prop.value == "off"){
          m_impl->getDevice()->used->irShift = FreenectDevice::Used::Off;
        }else if(prop.value == "fast"){
          m_impl->getDevice()->used->irShift = FreenectDevice::Used::Fast;
        }else if(prop.value == "accurate"){
          m_impl->getDevice()->used->irShift = FreenectDevice::Used::Accurate;
        }else{
          ERROR_LOG("invalid property value for property 'shift-IR-image':" << prop.value);
        }
      }else if(prop.name == "depth-image-unit"){
        if(prop.value == "mm") m_impl->getDevice()->used->depthImageUnitMM = true;
        else if(prop.value == "raw") m_impl->getDevice()->used->depthImageUnitMM = false;
        else{
          ERROR_LOG("invalid property value for property 'depth-image-unit':" << prop.value);
        }
      }else if(prop.name == "depth-image-post-processing"){
        if(prop.value == "off") m_impl->getDevice()->used->depthImagePostProcessingMedianRadius = 0;
        else if(prop.value == "median 3x3") m_impl->getDevice()->used->depthImagePostProcessingMedianRadius = 3;
        else if(prop.value == "median 5x5") m_impl->getDevice()->used->depthImagePostProcessingMedianRadius = 5;
        else{
          ERROR_LOG("invalid property value for property 'depth-image-post-processing':" << prop.value);
        }
      }
    }

    REGISTER_CONFIGURABLE(KinectGrabber, return new KinectGrabber(KinectGrabber::GRAB_DEPTH_IMAGE, 0, utils::Size::VGA));

    /// returns a list of attached kinect devices
    const std::vector<GrabberDeviceDescription> &KinectGrabber::getDeviceList(bool rescan){
      static std::vector<GrabberDeviceDescription> devices;
      if(rescan){
        devices.clear();
        FreenectContext &ctx = FreenectContext::getFreenectContext();
        
        for(size_t i=0;i<ctx.deviceAssociation.size();++i){
          try{
            KinectGrabber g(GRAB_RGB_IMAGE,i);
            
            std::string serial = ctx.deviceAssociation[i];
            std::string s = str(i);
            if(serial != "null"){            
              s += "|||" + serial;
            }
            devices.push_back(GrabberDeviceDescription("kinectd",s,"Kinect Depth Camera (ID "+str(i)+")"));
            devices.push_back(GrabberDeviceDescription("kinectc",s,"Kinect Color Camera RGB (ID "+str(i)+")"));
            devices.push_back(GrabberDeviceDescription("kinecti",s,"Kinect Color Camera IR (ID "+str(i)+")"));
          }catch(ICLException &e){
            (void)e;//SHOW(e.what());
            break;
          }
        }
      }
      return devices;
    }

    Grabber* createDepthGrabber(const std::string &param){
      return new KinectGrabber(KinectGrabber::GRAB_DEPTH_IMAGE,param);
    }

    Grabber* createRGBGrabber(const std::string &param){
      return new KinectGrabber(KinectGrabber::GRAB_RGB_IMAGE,param);
    }

    Grabber* createIRGrabber(const std::string &param){
      return new KinectGrabber(KinectGrabber::GRAB_IR_IMAGE_8BIT,param);
    }

    const std::vector<GrabberDeviceDescription>& getKinectDDeviceList(std::string hint, bool rescan){
      static std::vector<GrabberDeviceDescription> devices;
      if(rescan){
        FreenectContext &ctx = FreenectContext::getFreenectContext();
        devices.clear();
        int deviceCount = ctx.numDevices();
        for(int i = 0; i < deviceCount; ++i){
          std::string serial = ctx.deviceAssociation[i];
          std::string s = str(i);
          if(serial != "null"){            
            s += "|||" + serial;
          }
          devices.push_back(GrabberDeviceDescription("kinectd",s,"Kinect Depth Camera (ID "+str(i)+")"));
        }
      }
      return devices;
    }

    const std::vector<GrabberDeviceDescription>& getKinectCDeviceList(std::string hint, bool rescan){
      static std::vector<GrabberDeviceDescription> devices;
      if(rescan){
        FreenectContext &ctx = FreenectContext::getFreenectContext();
        devices.clear();
        int deviceCount = ctx.numDevices();
        for(int i = 0; i < deviceCount; ++i){
          std::string serial = ctx.deviceAssociation[i];
          std::string s = str(i);
          if(serial != "null"){            
            s += "|||" + serial;
          }
          devices.push_back(GrabberDeviceDescription("kinectc",s,"Kinect Color Camera RGB (ID "+str(i)+")"));
        }
      }
      return devices;
    }

    const std::vector<GrabberDeviceDescription>& getKinectIDeviceList(std::string hint, bool rescan){
      static std::vector<GrabberDeviceDescription> devices;
      if(rescan){
        FreenectContext &ctx = FreenectContext::getFreenectContext();
        devices.clear();
        int deviceCount = ctx.numDevices();
        for(int i = 0; i < deviceCount; ++i){
          std::string serial = ctx.deviceAssociation[i];
          std::string s = str(i);
          if(serial != "null"){            
            s += "|||" + serial;
          }
          devices.push_back(GrabberDeviceDescription("kinecti",s,"Kinect Color Camera IR (ID "+str(i)+")"));
        }
      }
      return devices;
    }

    REGISTER_GRABBER(kinectd,utils::function(createDepthGrabber), utils::function(getKinectDDeviceList), "kinectd:device ID:kinect depth camera source:");
    REGISTER_GRABBER(kinectc,utils::function(createRGBGrabber), utils::function(getKinectCDeviceList),"kinectc:device ID:kinect color camera source");
    REGISTER_GRABBER(kinecti,utils::function(createIRGrabber), utils::function(getKinectIDeviceList),"kinecti:devide ID:kinect IR camera source");

  } // namespace io
} // namespace icl

