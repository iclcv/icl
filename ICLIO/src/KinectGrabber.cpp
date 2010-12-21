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

#include <libfreenect.hpp>

namespace icl{
  namespace{
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

  struct KinectGrabber::Impl{
    ICLKinectDevice *device;
    KinectGrabber::Mode mode;
    int index;
    int ledColor;
    double desiredTiltDegrees;
    Mutex mutex;
  };
  
  KinectGrabber::KinectGrabber(KinectGrabber::Mode mode, int deviceID) throw (ICLException) :m_impl(new Impl){
    m_impl->mode = mode;
    m_impl->index = deviceID;
    if(mode == GRAB_RGB_IMAGE){
      m_impl->device = ICLKinectDeviceFactory::createDevice(deviceID,true);
    }else if(mode == GRAB_DEPTH_IMAGE){
      m_impl->device = ICLKinectDeviceFactory::createDevice(deviceID,false);
    }else{
      throw ICLException("KinectGrabber::KinectGrabber given mode is not supported yet");
    }
    m_impl->ledColor = 0;
    m_impl->desiredTiltDegrees = 0;
  }
  
  KinectGrabber::~KinectGrabber(){
    ICLKinectDeviceFactory::freeDevice(m_impl->index,m_impl->mode == GRAB_RGB_IMAGE);
    delete m_impl;
  }
  
  const ImgBase* KinectGrabber::grabUD(ImgBase **dst){
    Mutex::Locker lock(m_impl->mutex);
    if(m_impl->mode == GRAB_RGB_IMAGE){
      return adaptGrabResult(&m_impl->device->getLastColorImage(),dst);
    }else{
      return adaptGrabResult(&m_impl->device->getLastDepthImage(),dst);
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
      return m_impl->mode == GRAB_RGB_IMAGE ? "Color Image (24Bit RGB)" : "Depth Image (float)";
    }else if(name == "size"){
      return "VGA (640x480)";
    }else if(name == "LED"){
      return str(m_impl->ledColor);
    }else if(name == "Desired-Tilt-Angle"){
      return str(m_impl->desiredTiltDegrees);
    }else if(name == "Current-Tilt-Angle"){
      //      m_impl->device->updateState();
      return str(m_impl->device->getState().getTiltDegs());
    }else if( name == "Accelerometers"){
      double a[3]={0,0,0};
      //m_impl->device->updateState();
      m_impl->device->getState().getAccelerometers(a,a+1,a+2);
      return str(a[0]) + "," + str(a[1]) + "," + str(a[2]);
    }else{
      return "undefined";
    }

  }

  /// Returns whether this property may be changed internally
  int KinectGrabber::isVolatile(const std::string &name){
    Mutex::Locker lock(m_impl->mutex);
    return (name == "Desired-Tilt-Angle" || name == "Accelerometers") * 50;
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
      if(m_impl->mode != newMode){
        m_impl->device->removeUser(m_impl->mode == GRAB_RGB_IMAGE);
        m_impl->device->addUser(newMode == GRAB_RGB_IMAGE);
        m_impl->mode = newMode;
      }
    }else if(name == "size"){
      if(value != "VGA (640x480)"){
        ERROR_LOG("invalid property value for property 'size'");
      }
    }else if(name == "LED"){
      if(value == "off"){
        m_impl->device->setLed((freenect_led_options)0);
      }else if(value == "green"){
        m_impl->device->setLed((freenect_led_options)1);
      }else if(value == "red"){
        m_impl->device->setLed((freenect_led_options)2);
      }else if(value == "yellow"){
        m_impl->device->setLed((freenect_led_options)3);
      }else if(value == "blink yellow"){
        m_impl->device->setLed((freenect_led_options)4);
      }else if(value == "blink green"){
        m_impl->device->setLed((freenect_led_options)5);
      }else if(value == "blink red/yellow"){
        m_impl->device->setLed((freenect_led_options)6);
      }else{
        ERROR_LOG("invalid property value for property 'LED'");
      }
    }else if(name == "Desired-Tilt-Angle"){
      m_impl->device->setTiltDegrees(parse<double>(value));
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
  
  const ImgBase *KinectGrabber::adaptGrabResult(const ImgBase *src, ImgBase **dst){
    if(!getIgnoreDesiredParams()){ // use desired parameters
      if(src->getDepth() == getDesiredDepth() && src->getSize() == getDesiredSize() && src->getFormat() == getDesiredFormat()){
        // by chance: desired parameters are correctly
        if(dst){
          src->deepCopy(dst);
          return *dst;
        }else{
          return src;
        }
      }else{
        if(dst){
          ensureCompatible(dst,getDesiredDepth(),getDesiredParams());
          m_oConverter.apply(src,*dst);
          return *dst;
        }else{
          m_poImage = imgNew(getDesiredDepth(), getDesiredParams());
          m_oConverter.apply(src,m_poImage);
          return m_poImage;
        }
      }
    }else{ // no desired parameters ...
      if(dst){
        src->deepCopy(dst);
        return *dst;
      }else{
        return src;
      }
    }
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
        }catch(...){
          break;
        }
      }
    }
    return devices;
  }

}


#endif
