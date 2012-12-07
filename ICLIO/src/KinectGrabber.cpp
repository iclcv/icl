/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/KinectGrabber.cpp                            **
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

#ifdef HAVE_LIBFREENECT

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

    struct FreenectContext : public Thread{
        freenect_context *ctx;
        int errors;
        static const int MAX_ERRORS = 100;
        FreenectContext():errors(0){
          if(freenect_init(&ctx, NULL) < 0){
            throw ICLException("unable to create freenect_context");
          }
        }
        ~FreenectContext(){
          stop();
          freenect_shutdown(ctx);
        }
        virtual void run(){
          while(true){
            if(freenect_process_events(ctx) < 0){
              errors++;
            }
            if(errors > MAX_ERRORS){
              throw ICLException("detected 100th error in freenect event processing");
            }
          }
          Thread::msleep(20);
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

            Used():postProcessor3x3(Size(3,3)),postProcessor5x5(Size(5,5)){
              postProcessor3x3.setClipToROI(false);
              postProcessor5x5.setClipToROI(false);
            }

            KinectGrabber::Mode currentColorMode; // this must not be reset as long as the device is used somewhere else


            void depth_cb(void *data, uint32_t timestamp){
              Mutex::Locker lock(depthMutex);
              const int r = depthImagePostProcessingMedianRadius;
              MedianOp *pp = (r == 3) ? &postProcessor3x3 : r == 5 ? &postProcessor5x5 : (MedianOp*)0;
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
        }

        FreenectDevice(FreenectContext &ctx, int index, KinectGrabber::Mode mode, Size size):mode(mode),index(index){
          std::map<int,Used*>::iterator it = devices.find(index);
          if(it == devices.end()){
            //        DEBUG_LOG("device " << index << " was not used before: creating new one");
            used = devices[index] = new Used;
            used->irShift = Used::Accurate;
            used->depthImageUnitMM = true;
            used->currentColorMode = mode;
            used->depthImagePostProcessingMedianRadius = 0;

            if(freenect_open_device(ctx.ctx, &used->device, index) < 0){
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
              used->numDepthUsers++;

              setMode(mode, used, size);

              used->depthImage = Img32f(size,formatMatrix);
              freenect_set_depth_callback(used->device,freenect_depth_callback);
              if(freenect_start_depth(used->device) < 0){
                throw ICLException("FreenectDevice:: unable to start depth for device" + str(index));
              }
            }
          }else{ // reuse old device
            // DEBUG_LOG("device " << index << " was used before: using old one");
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
    
    
    
    struct KinectGrabber::Impl{

        static SmartPtr<FreenectContext> context;
        SmartPtr<FreenectDevice> device;
        int ledColor;
        float desiredTiltDegrees;
        bool avoidDoubleFrames;
        Mutex mutex;
        Time lastupdate;

        Impl(KinectGrabber::Mode mode, int index, const Size &size)
          : mutex(Mutex::mutexTypeRecursive), lastupdate(Time::now())
        {
          Mutex::Locker lock(mutex);

          bool createdContextHere = false;
          if(!context){
            createdContextHere = true;
            context = SmartPtr<FreenectContext>(new FreenectContext,false); // hack do not pass ownership to the smartpointer
            // because freeing the freenect context did not work
          }
          device = SmartPtr<FreenectDevice>(new FreenectDevice(*context,index,mode, size));
          if(createdContextHere){
            context->start();
          }

        }

        void switchMode(KinectGrabber::Mode mode, const Size &size){
          if(device->mode != mode || device->used->size != size){
            int idx = device->index;
            device = SmartPtr<FreenectDevice>();
            device = SmartPtr<FreenectDevice>(new FreenectDevice(*context,idx,mode,size));
          }
        }

        inline const Img32f &getLastDepthImage(){
          return device->used->getLastDepthImage(avoidDoubleFrames);
        }
        inline const ImgBase &getLastColorImage(){
          return device->used->getLastColorImage(avoidDoubleFrames);
        }
    };
    

    std::map<int,FreenectDevice::Used*> FreenectDevice::devices;
    SmartPtr<FreenectContext> KinectGrabber::Impl::context;

    KinectGrabber::KinectGrabber(KinectGrabber::Mode mode, int deviceID, const Size &size) throw (ICLException):
      m_impl(new Impl(mode,deviceID,size))

    {
      m_impl->ledColor = 0;
      m_impl->desiredTiltDegrees = 0;
      m_impl->avoidDoubleFrames  = true;

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

      try{m_impl->device->used->updateState();}catch(...){}
      double degs = m_impl->device->used->getState().getTiltDegs();
      std::string angleval = (degs == -64) ? "moving" : str(degs);
      double a[3]={0,0,0};
      m_impl->device->used->getState().getAccelerometers(a,a+1,a+2);
      std::string accelval = str(a[0]) + "-" + str(a[1]) + "-" + str(a[2]);
      std::string diunit = (m_impl->device->used->depthImageUnitMM) ? "mm" : "raw";
      const int r  = m_impl->device->used->depthImagePostProcessingMedianRadius;
      std::string ppvalue = (r == 3) ? "median 3x3" : ((r == 5) ? "median 5x4" : "off");

      addProperty("format", "menu", "Color Image {24Bit RGB},Depth Image {float},IR Image {8Bit},IR Image {10Bit}", formats[m_impl->device->mode], 0, "");
      addProperty("size", "menu", "VGA {640x480}", "VGA {640x480}", 0, "");
      addProperty("LED", "menu", "off,green,red,yellow,blink yellow,blink green,blink red/yellow", m_impl->ledColor, 0, "");
      addProperty("Desired-Tilt-Angle", "range", "[-35,25]", m_impl->desiredTiltDegrees, 0, "");
      addProperty("Current-Tilt-Angle", "info", "", angleval, 200, "");
      addProperty("Accelerometers", "info", "", accelval, 200, "");
      addProperty("shift-IR-image", "menu", "off,fast,accurate", values[(int)(m_impl->device->used->irShift)], 0, "");
      addProperty("depth-image-unit", "menu", "raw,mm", diunit, 0, "");
      addProperty("depth-image-post-processing", "menu", "off,median 3x3,median 5x5", ppvalue, 0, "");

      Configurable::registerCallback(utils::function(this,&KinectGrabber::processPropertyChange));
    }
    
    KinectGrabber::~KinectGrabber(){
      delete m_impl;
    }
    
    const ImgBase* KinectGrabber::acquireImage(){
      Mutex::Locker lock(m_impl->mutex);
      // update current angle and accelometers every 200ms
      if(m_impl -> lastupdate.age() > 200000){
        Time t = Time::now();
        try{m_impl->device->used->updateState();}catch(...){DEBUG_LOG("could not update")}
        double degs = m_impl->device->used->getState().getTiltDegs();
        std::string angleval = (degs == -64) ? "moving" : str(degs);
        double a[3]={0,0,0};
        m_impl->device->used->getState().getAccelerometers(a,a+1,a+2);
        std::string accelval = str(a[0]) + "-" + str(a[1]) + "-" + str(a[2]);
        setPropertyValue("Current-Tilt-Angle", angleval);
        setPropertyValue("Accelerometers", accelval);
        m_impl -> lastupdate = Time::now();
      }

      if(m_impl->device->mode != GRAB_DEPTH_IMAGE){
        return &m_impl->getLastColorImage();
      }else{
        return &m_impl->getLastDepthImage();
      }
    }

    /// callback for changed configurable properties
    void KinectGrabber::processPropertyChange(const utils::Configurable::Property &prop){
      Mutex::Locker lock(m_impl->mutex);

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
        m_impl->switchMode((Mode)idx, m_impl->device->used->size);

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
          m_impl->device->used->setLed((freenect_led_options)0);
        }else if(prop.value == "green"){
          m_impl->device->used->setLed((freenect_led_options)1);
        }else if(prop.value == "red"){
          m_impl->device->used->setLed((freenect_led_options)2);
        }else if(prop.value == "yellow"){
          m_impl->device->used->setLed((freenect_led_options)3);
        }else if(prop.value == "blink yellow"){
          m_impl->device->used->setLed((freenect_led_options)4);
        }else if(prop.value == "blink green"){
          m_impl->device->used->setLed((freenect_led_options)5);
        }else if(prop.value == "blink red/yellow"){
          m_impl->device->used->setLed((freenect_led_options)6);
        }else{
          ERROR_LOG("invalid property value for property 'LED'" << prop.value);
        }
      }else if(prop.name == "Desired-Tilt-Angle"){
        m_impl->device->used->setTiltDegrees(parse<double>(prop.value));
      }else if(prop.name == "shift-IR-image"){
        if(prop.value == "off"){
          m_impl->device->used->irShift = FreenectDevice::Used::Off;
        }else if(prop.value == "fast"){
          m_impl->device->used->irShift = FreenectDevice::Used::Fast;
        }else if(prop.value == "accurate"){
          m_impl->device->used->irShift = FreenectDevice::Used::Accurate;
        }else{
          ERROR_LOG("invalid property value for property 'shift-IR-image':" << prop.value);
        }
      }else if(prop.name == "depth-image-unit"){
        if(prop.value == "mm") m_impl->device->used->depthImageUnitMM = true;
        else if(prop.value == "raw") m_impl->device->used->depthImageUnitMM = false;
        else{
          ERROR_LOG("invalid property value for property 'depth-image-unit':" << prop.value);
        }
      }else if(prop.name == "depth-image-post-processing"){
        if(prop.value == "off") m_impl->device->used->depthImagePostProcessingMedianRadius = 0;
        else if(prop.value == "median 3x3") m_impl->device->used->depthImagePostProcessingMedianRadius = 3;
        else if(prop.value == "median 5x5") m_impl->device->used->depthImagePostProcessingMedianRadius = 5;
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
        for(int i=0;i<8;++i){
          try{
            KinectGrabber g(GRAB_RGB_IMAGE,i);
            devices.push_back(GrabberDeviceDescription("kinectd",str(i),"Kinect Depth Camera (ID "+str(i)+")"));
            devices.push_back(GrabberDeviceDescription("kinectc",str(i),"Kinect Color Camera RGB (ID "+str(i)+")"));
            devices.push_back(GrabberDeviceDescription("kinecti",str(i),"Kinect Color Camera IR (ID "+str(i)+")"));
          }catch(ICLException &e){
            (void)e;//SHOW(e.what());
            break;
          }
        }
      }
      return devices;
    }

  } // namespace io
}


#endif



#if 0
// adpated old version using libfreenect.hpp
struct ICLKinectDevice : public Freenect::FreenectDevice{
    ICLKinectDevice(freenect_context *ctx, int index):
      Freenect::FreenectDevice(ctx,index),m_ctx(ctx),
      m_index(index){
      colorImage = Img8u(Size::VGA,formatRGB);
      depthImage = Img32f(Size::VGA,1);
      avoidDoubleFrames = true;
      colorOn = false;
      depthOn = false;

      m_userCount[0] = m_userCount[1] = 0;
    }

    ~ICLKinectDevice(){
      stopColorICL();
      stopDepthICL();
    }

    int getIndex() const { return m_index; }

    void VideoCallback(void *data, uint32_t timestamp){
      Mutex::Locker lock(m_colorMutex);
      colorImage.setTime(Time::now());
      interleavedToPlanar((const icl8u*)data,&colorImage);
    }

    void DepthCallback(void *data, uint32_t timestamp){
      Mutex::Locker lock(m_depthMutex);
      depthImage.setTime(Time::now());
      std::copy((const icl16s*)data,(const icl16s*)data+640*480,depthImage.begin(0));
    }

    const Img32f &getLastDepthImage(){
      Mutex::Locker lock(m_depthMutex);
      if(avoidDoubleFrames){
        while(lastDepthTime == depthImage.getTime()){
          m_depthMutex.unlock();
          Thread::msleep(1);
          m_depthMutex.lock();
        }
      }
      lastDepthTime = depthImage.getTime();
      depthImage.deepCopy(&depthImageOut);

      return depthImageOut;
    }
    const Img8u &getLastColorImage(){
      Mutex::Locker lock(m_colorMutex);
      if(avoidDoubleFrames){
        while(lastColorTime == colorImage.getTime()){
          m_colorMutex.unlock();
          Thread::msleep(1);
          m_colorMutex.lock();
        }
      }
      lastColorTime = colorImage.getTime();
      colorImage.deepCopy(&colorImageOut);
      return colorImageOut;
    }

    void startColorICL(){
      if(!colorOn){
        startVideo();
        colorOn = true;
      }
    }

    void startDepthICL(){
      if(!depthOn){
        startDepth();
        depthOn = true;
      }
    }

    void stopColorICL(){
      if(colorOn){
        stopVideo();
        colorOn = false;
      }
    }

    void stopDepthICL(){
      if(depthOn){
        stopDepth();
        depthOn = false;
      }
    }

    void startICL(bool color){
      if(color) startColorICL();
      else startDepthICL();
    }

    void stopICL(bool color){
      if(color) stopColorICL();
      else stopDepthICL();
    }

    void addUser(bool color){
      ++m_userCount[color?1:0];
    }

    void removeUser(bool color){
      --m_userCount[color?1:0];
    }

    bool hasUsers(bool color) const{
      return m_userCount[color?1:0];
    }
    bool hasUsers() const{
      return hasUsers(true) || hasUsers(false);
    }

  protected:
    freenect_context *m_ctx;
    int m_index,m_userCount[2];
    Mutex m_colorMutex, m_depthMutex;
    Img32f depthImage,depthImageOut;
    Img8u colorImage,colorImageOut;
    bool avoidDoubleFrames;
    Time lastColorTime, lastDepthTime;
    bool colorOn, depthOn;
};



class ICLKinectDeviceFactory{
    ICLKinectDeviceFactory(){}
    static SmartPtr<Freenect::Freenect<ICLKinectDevice> > singelton;
    static std::map<int,ICLKinectDevice*> deviceMap;
  public:
    static Freenect::Freenect<ICLKinectDevice> &factory() {
      if(!singelton) {
        singelton = SmartPtr<Freenect::Freenect<ICLKinectDevice> >(new Freenect::Freenect<ICLKinectDevice>);
      }
      return *singelton;
    }

    static ICLKinectDevice *createDevice(int index, bool color){
      ICLKinectDevice *&dev = deviceMap[index];
      if(!dev){
        try{
          dev = &factory().createDevice(index);
        }catch(const std::runtime_error &err){
          throw ICLException("Unable to get access to Kinect device with ID " + str(index));
        }
      }
      dev->addUser(color);
      dev->startICL(color);
      return dev;
    }

    static void freeDevice(int index, int color){
      std::map<int,ICLKinectDevice*>::iterator it = deviceMap.find(index);
      if(it == deviceMap.end()) throw ICLException("unable to free an unused kinect device");
      ICLKinectDevice *&dev = it->second;
      dev->stopICL(color);
      dev->removeUser(color);
      if(!dev->hasUsers()){
        delete dev;
        deviceMap.erase(it);
      }
    }
};


SmartPtr<Freenect::Freenect<ICLKinectDevice> > ICLKinectDeviceFactory::singelton;
std::map<int,ICLKinectDevice*> ICLKinectDeviceFactory::deviceMap;
}



#endif




#if 0
namespace{
  struct ICLFreenectDevice : Freenect::FreenectDevice{
      Mutex m_colorMutex, m_depthMutex;
      Img32f depthImage,depthImageOut;
      Img8u colorImage,colorImageOut;
      bool avoidDoubleFrames;
      Time lastColorTime, lastDepthTime;
      int referenceCount;
      
      ICLFreenectDevice(freenect_context *_ctx, int _index): FreenectDevice(_ctx,_index){
        referenceCount = 1;
        colorImage = Img8u(Size::VGA,formatRGB);
        depthImage = Img32f(Size::VGA,1);
        avoidDoubleFrames = true;
      }
      
      void VideoCallback(void *data, uint32_t timestamp){
        Mutex::Locker lock(m_colorMutex);
        colorImage.setTime(Time::now());
        interleavedToPlanar((const icl8u*)data,&colorImage);
      }
      
      void DepthCallback(void *data, uint32_t timestamp){
        Mutex::Locker lock(m_depthMutex);
        depthImage.setTime(Time::now());
        std::copy((const icl16s*)data,(const icl16s*)data+640*480,depthImage.begin(0));
      }
      const Img32f &getLastDepthImage(){
        Mutex::Locker lock(m_depthMutex);
        if(avoidDoubleFrames){
          while(lastDepthTime == depthImage.getTime()){
            m_depthMutex.unlock();
            Thread::msleep(1);
            m_depthMutex.lock();
          }
        }
        lastDepthTime = depthImage.getTime();
        depthImage.deepCopy(&depthImageOut);
        
        return depthImageOut;
      }
      const Img8u &getLastColorImage(){
        Mutex::Locker lock(m_colorMutex);
        if(avoidDoubleFrames){
          while(lastColorTime == colorImage.getTime()){
            m_colorMutex.unlock();
            Thread::msleep(1);
            m_colorMutex.lock();
          }
        }
        lastColorTime = colorImage.getTime();
        colorImage.deepCopy(&colorImageOut);
        return colorImageOut;
      }
  };
} // anonymos namespace
#endif

