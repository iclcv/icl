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

#include <libfreenect.h>
#include <map>
namespace icl{
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
      freenect_device *device;
      int numColorUsers;
      int numDepthUsers;
      Mutex colorMutex,depthMutex;
      Img8u colorImage,colorImageOut;
      Img32f depthImage,depthImageOut;  
      Time lastColorTime, lastDepthTime;
      
      void depth_cb(void *data, uint32_t timestamp){
        Mutex::Locker lock(depthMutex);
        depthImage.setTime(Time::now());
        std::copy((const icl16s*)data,(const icl16s*)data+640*480,depthImage.begin(0)); 
      }
      void color_cb(void *data, uint32_t timestamp){
        Mutex::Locker lock(colorMutex);
        colorImage.setTime(Time::now());
        interleavedToPlanar((const icl8u*)data,&colorImage);
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
      const Img8u &getLastColorImage(bool avoidDoubleFrames){
        Mutex::Locker lock(colorMutex);
        if(avoidDoubleFrames){
          while(lastColorTime == colorImage.getTime()){
            colorMutex.unlock();
            Thread::msleep(1);
            colorMutex.lock();
          }
        }
        lastColorTime = colorImage.getTime();
        colorImage.deepCopy(&colorImageOut);
        return colorImageOut;
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
    
    FreenectDevice(FreenectContext &ctx, int index, KinectGrabber::Mode mode):mode(mode),index(index){
      std::map<int,Used*>::iterator it = devices.find(index);
      if(it == devices.end()){
        //        DEBUG_LOG("device " << index << " was not used before: creating new one");
        used = devices[index] = new Used;

        if(freenect_open_device(ctx.ctx, &used->device, index) < 0){
          throw ICLException("FreenectDevice:: unable to open kinect device for device " + str(index));
        }
        used->numColorUsers = used->numDepthUsers = 0;
        freenect_set_user(used->device, used);
        
        if(mode == KinectGrabber::GRAB_RGB_IMAGE){
          used->numColorUsers++;
          freenect_set_video_format(used->device, FREENECT_VIDEO_RGB);
          used->colorImage = Img8u(Size::VGA,formatRGB);
          freenect_set_video_callback(used->device,freenect_video_callback);
          if(freenect_start_video(used->device) < 0){
            throw ICLException("FreenectDevice:: unable to start video for device" + str(index));
          }
        }else{
          used->numDepthUsers++;
          freenect_set_depth_format(used->device, FREENECT_DEPTH_11BIT);
          used->depthImage = Img32f(Size::VGA,formatMatrix);
          freenect_set_depth_callback(used->device,freenect_depth_callback);
          if(freenect_start_depth(used->device) < 0){
            throw ICLException("FreenectDevice:: unable to start depth for device" + str(index));
          }
        }
      }else{
        // DEBUG_LOG("device " << index << " was used before: using old one");
        used = it->second;
        if(mode == KinectGrabber::GRAB_RGB_IMAGE){
          if(!used->numColorUsers){
            freenect_set_video_format(used->device, FREENECT_VIDEO_RGB);
            used->colorImage = Img8u(Size::VGA,formatRGB);
            freenect_set_video_callback(used->device,freenect_video_callback);
            if(freenect_start_video(used->device) < 0){
              throw ICLException("FreenectDevice:: unable to start video for device" + str(index));
            }
          }
          used->numColorUsers++;
        }else{
          if(!used->numDepthUsers){
            freenect_set_depth_format(used->device, FREENECT_DEPTH_11BIT);
            used->depthImage = Img32f(Size::VGA,formatMatrix);
            freenect_set_depth_callback(used->device,freenect_depth_callback);
            if(freenect_start_depth(used->device) < 0){
              throw ICLException("FreenectDevice:: unable to start depth for device" + str(index));
            }
          }          
        }
      }
    }
    ~FreenectDevice(){
      if(mode == KinectGrabber::GRAB_RGB_IMAGE){
        used->numColorUsers--;
        if(!used->numColorUsers){
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
    
    Impl(KinectGrabber::Mode mode, int index){
      Mutex::Locker lock(mutex);
      bool createdContextHere = false;
      if(!context){
        createdContextHere = true;
        context = SmartPtr<FreenectContext>(new FreenectContext);
      }
      device = SmartPtr<FreenectDevice>(new FreenectDevice(*context,index,mode));
      if(createdContextHere){
        context->start();
      }
    }

    void switchMode(KinectGrabber::Mode mode){
      if(device->mode != mode){
        device = SmartPtr<FreenectDevice>(new FreenectDevice(*context,device->index,mode));
      }
    }
    
    inline const Img32f &getLastDepthImage(){
      return device->used->getLastDepthImage(avoidDoubleFrames);
    }
    inline const Img8u &getLastColorImage(){
      return device->used->getLastColorImage(avoidDoubleFrames);
    }
  };
  
    
  std::map<int,FreenectDevice::Used*> FreenectDevice::devices; 
  SmartPtr<FreenectContext> KinectGrabber::Impl::context;

  KinectGrabber::KinectGrabber(KinectGrabber::Mode mode, int deviceID) throw (ICLException):
    m_impl(new Impl(mode,deviceID)){
    m_impl->ledColor = 0;
    m_impl->desiredTiltDegrees = 0;
    m_impl->avoidDoubleFrames  = true;
  }
  
  KinectGrabber::~KinectGrabber(){
    delete m_impl;
  }
  
  const ImgBase* KinectGrabber::grabUD(ImgBase **dst){
    Mutex::Locker lock(m_impl->mutex);
    if(m_impl->device->mode == GRAB_RGB_IMAGE){
      return adaptGrabResult(&m_impl->getLastColorImage(),dst);
    }else{
      return adaptGrabResult(&m_impl->getLastDepthImage(),dst);
    }
  }
  
  /// get type of property 
  std::string KinectGrabber::getType(const std::string &name){
    Mutex::Locker lock(m_impl->mutex);
    if(name == "format" || name == "size" || name == "LED") return "menu";
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
      return "{\"Color Image (24Bit RGB)\",\"Depth Image (float)\"}";
    }else if(name == "size"){
      return "{\"VGA (640x480)\"}";
    }else if(name == "LED"){
      return "{\"off\",\"green\",\"red\",\"yellow\",\"blink yellow\",\"blink green\",\"blink red/yellow\"}";
    }else if(name == "Desired-Tilt-Angle"){
      return "[-35,25]";
    }else if(name == "Current-Tilt-Angle" || name == "Accelerometers"){
      return "undefined for type info!";
    }else{
      return "undefined";
    }
  }
  
  /// returns the current value of a property or a parameter
  std::string KinectGrabber::getValue(const std::string &name){
    Mutex::Locker lock(m_impl->mutex);
    if(name == "format"){
      return m_impl->device->mode == GRAB_RGB_IMAGE ? "Color Image (24Bit RGB)" : "Depth Image (float)";
    }else if(name == "size"){
      return "VGA (640x480)";
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
      if(value != "Color Image (24Bit RGB)" && value != "Depth Image (float)"){
        ERROR_LOG("invalid property value for property 'format'");
        return;
      }
      Mode newMode = value == "Color Image (24Bit RGB)" ? GRAB_RGB_IMAGE : GRAB_DEPTH_IMAGE;
      m_impl->switchMode(newMode);
        
    }else if(name == "size"){
      if(value != "VGA (640x480)"){
        ERROR_LOG("invalid property value for property 'size'");
      }
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
        ERROR_LOG("invalid property value for property 'LED'");
      }
    }else if(name == "Desired-Tilt-Angle"){
      m_impl->device->used->setTiltDegrees(parse<double>(value));
    }else if(name == "Current-Tilt-Angle" || name == "Accelerometers"){
      ERROR_LOG("info-properties cannot be set");
    }else{
      ERROR_LOG("invalid property name '" << name << "'"); 
    }
  }
  
  /// returns a list of properties, that can be set using setProperty
  std::vector<std::string> KinectGrabber::getPropertyList(){
    Mutex::Locker lock(m_impl->mutex);
    static std::string props[] = {"format","size","LED","Desired-Tilt-Angle","Current-Tilt-Angle","Accelerometers"};
    return std::vector<std::string>(props,props+6);
  }

  /// returns a list of attached kinect devices
  const std::vector<GrabberDeviceDescription> &KinectGrabber::getDeviceList(bool rescan){
    static std::vector<GrabberDeviceDescription> devices;
    if(rescan){
      devices.clear();
      for(int i=0;i<8;++i){
        try{
          KinectGrabber g(GRAB_RGB_IMAGE,i);
          devices.push_back(GrabberDeviceDescription("Kinect",str(i),"Kinect Device (ID "+str(i)+")"));
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
 
