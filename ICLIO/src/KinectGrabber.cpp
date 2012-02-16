/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/KinectGrabber.cpp                            **
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

#ifdef HAVE_LIBFREENECT

#include <ICLIO/KinectGrabber.h>
#include <ICLCC/CCFunctions.h>
#include <ICLUtils/StringUtils.h>
#include <ICLCore/Img.h>
#include <ICLUtils/Mutex.h>
#include <ICLUtils/Thread.h>
#include <ICLFilter/TranslateOp.h>

#include <libfreenect.h>
#include <map>
namespace icl{

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
    friend class FreenectDevice;
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
      KinectGrabber::Mode currentColorMode; // this must not be reset as long as the device is used somewhere else
      
      
      void depth_cb(void *data, uint32_t timestamp){
        Mutex::Locker lock(depthMutex);
        depthImage.setTime(Time::now());
        std::copy((const icl16s*)data,(const icl16s*)data+640*480,depthImage.begin(0)); 
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
            throw ICLException("processed color callback for depth grabber (this should not happen)");
        }

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
        depthImage.deepCopy(&depthImageOut);
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
        used->currentColorMode = mode;

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
    
    Impl(KinectGrabber::Mode mode, int index, const Size &size){
      Mutex::Locker lock(mutex);

      bool createdContextHere = false;
      if(!context){
        createdContextHere = true;
        context = SmartPtr<FreenectContext>(new FreenectContext);
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
    m_impl(new Impl(mode,deviceID,size)){
    m_impl->ledColor = 0;
    m_impl->desiredTiltDegrees = 0;
    m_impl->avoidDoubleFrames  = true;
  }
  
  KinectGrabber::~KinectGrabber(){
    delete m_impl;
  }
  
  const ImgBase* KinectGrabber::acquireImage(){
    Mutex::Locker lock(m_impl->mutex);
    if(m_impl->device->mode != GRAB_DEPTH_IMAGE){
      return &m_impl->getLastColorImage();
    }else{
      return &m_impl->getLastDepthImage();
    }
  }
  
  /// get type of property 
  std::string KinectGrabber::getType(const std::string &name){
    Mutex::Locker lock(m_impl->mutex);
    if(name == "format" || name == "size" || name == "LED" || name == "shift-IR-image") return "menu";
    else if(name == "Desired-Tilt-Angle"){
      return "range";
    }else if(name =="Current-Tilt-Angle" || "Accelerometers"){
      return "info";
    }else{
      return "undefined";
    }
  }
  
  /// get information of a properties valid values
  std::string KinectGrabber::getInfo(const std::string &name){
    Mutex::Locker lock(m_impl->mutex);
    if(name == "format"){
      //return "{\"Color Image (24Bit RGB)\",\"Depth Image (float)\",\"Bayer Image (8Bit)\",\"IR Image (8Bit)\",\"IR Image (10Bit)\"}";
      return "{\"Color Image (24Bit RGB)\",\"Depth Image (float)\",\"IR Image (8Bit)\",\"IR Image (10Bit)\"}";
    }else if(name == "size"){
      //return "{\"QVGA (320x240)\",\"VGA (640x480)\"}"; // todo
      return "{\"VGA (640x480)\"}"; // todo
    }else if(name == "LED"){
      return "{\"off\",\"green\",\"red\",\"yellow\",\"blink yellow\",\"blink green\",\"blink red/yellow\"}";
    }else if(name == "Desired-Tilt-Angle"){
      return "[-35,25]";
    }else if(name == "Current-Tilt-Angle" || name == "Accelerometers"){
      return "undefined for type info!";
    }else if(name == "shift-IR-image"){
      return "{\"off\",\"fast\",\"accurate\"}";
    }else{
      return "undefined";
    }
  }
  
  /// returns the current value of a property or a parameter
  std::string KinectGrabber::getValue(const std::string &name){
    Mutex::Locker lock(m_impl->mutex);
    if(name == "format"){
      static const std::string formats[] = {
        "Color Image (24Bit RGB)",
        "Bayer Image (8Bit)",
        "Depth Image (float)",
        "IR Image (8Bit)",
        "IR Image (10Bit)" 
      };
      return formats[m_impl->device->mode];
    }else if(name == "size"){
      return m_impl->device->used->size == Size::VGA ? "VGA (640x480)" : "QVGA (320x240)"; // todo
    }else if(name == "LED"){
      return str(m_impl->ledColor);
    }else if(name == "Desired-Tilt-Angle"){
      return str(m_impl->desiredTiltDegrees);
    }else if(name == "Current-Tilt-Angle"){
      try{
        m_impl->device->used->updateState();
      }catch(...){}
      double degs = m_impl->device->used->getState().getTiltDegs();
      if(degs == -64) return "moving";
      return str(degs);
    }else if( name == "Accelerometers"){
      try{
        m_impl->device->used->updateState();
      }catch(...){}
      double a[3]={0,0,0};
      m_impl->device->used->getState().getAccelerometers(a,a+1,a+2);
      return str(a[0]) + "," + str(a[1]) + "," + str(a[2]);
    }else if(name == "shift-IR-image"){
      static const std::string values[] = {"off","fast","accurate"};
      return values[(int)(m_impl->device->used->irShift)];
    }else{
      return "undefined";
    }

  }

  /// Returns whether this property may be changed internally
  int KinectGrabber::isVolatile(const std::string &name){
    Mutex::Locker lock(m_impl->mutex);
    return (name == "Current-Tilt-Angle" || name == "Accelerometers")  ? 200 : 0;
  }
  
  /// Sets a specific property value
  void KinectGrabber::setProperty(const std::string &name, const std::string &value){
    Mutex::Locker lock(m_impl->mutex);
    
    
    if(name == "format"){
      static const std::string formats[] = {
        "Color Image (24Bit RGB)",
        "Bayer Image (8Bit)",
        "Depth Image (float)",
        "IR Image (8Bit)",
        "IR Image (10Bit)" 
      };
      
      int idx = (int)(std::find(formats, formats+5, value) - formats);
      if(idx == 5){
        ERROR_LOG("invalid property value for property 'format'");
        return;
      }
      m_impl->switchMode((Mode)idx, m_impl->device->used->size);
        
    }else if(name == "size"){
      /*
          if(value != "VGA (640x480)" && value != "QVGA (320x240)"){
        ERROR_LOG("invalid property value for property 'size'");
          }else{
          m_impl->switchMode(m_impl->device->mode,value == "VGA (640x480)" ?
          Size::VGA : Size::QVGA);
          }
      */
    }else if(name == "LED"){
      if(value == "off"){
        m_impl->device->used->setLed((freenect_led_options)0);
      }else if(value == "green"){
        m_impl->device->used->setLed((freenect_led_options)1);
      }else if(value == "red"){
        m_impl->device->used->setLed((freenect_led_options)2);
      }else if(value == "yellow"){
        m_impl->device->used->setLed((freenect_led_options)3);
      }else if(value == "blink yellow"){
        m_impl->device->used->setLed((freenect_led_options)4);
      }else if(value == "blink green"){
        m_impl->device->used->setLed((freenect_led_options)5);
      }else if(value == "blink red/yellow"){
        m_impl->device->used->setLed((freenect_led_options)6);
      }else{
        ERROR_LOG("invalid property value for property 'LED'" << value);
      }
    }else if(name == "Desired-Tilt-Angle"){
      m_impl->device->used->setTiltDegrees(parse<double>(value));
    }else if(name == "Current-Tilt-Angle" || name == "Accelerometers"){
      ERROR_LOG("info-properties cannot be set");
    }else if(name == "shift-IR-image"){
      if(value == "off"){
        m_impl->device->used->irShift = FreenectDevice::Used::Off;
      }else if(value == "fast"){
        m_impl->device->used->irShift = FreenectDevice::Used::Fast;
      }else if(value == "accurate"){
        m_impl->device->used->irShift = FreenectDevice::Used::Accurate;
      }else{
        ERROR_LOG("invalid property value for property 'shift-IR-image':" << value);
      }
    }else{
      ERROR_LOG("invalid property name '" << name << "'"); 
    }
  }
  
  /// returns a list of properties, that can be set using setProperty
  std::vector<std::string> KinectGrabber::getPropertyList(){
    Mutex::Locker lock(m_impl->mutex);
    static std::string props[] = {"format","size","LED","Desired-Tilt-Angle","Current-Tilt-Angle","Accelerometers","shift-IR-image"};
    return std::vector<std::string>(props,props+7);
  }

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
 
