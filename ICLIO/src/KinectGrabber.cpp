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
** Authors: Christof Elbrechter, Andreas Raster                    **
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

namespace icl{
  struct ICLFreenectContext : public Thread{
    freenect_context *ctx;
    int errors;
    static const int MAX_ERRORS = 100;
    ICLFreenectContext():errors(0){
      if(freenect_init(&ctx, NULL) < 0){
        throw ICLException("unable to create freenect_context");
      }
    }
    ~ICLFreenectContext(){
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
        Thread::msleep(1);
      }
    }

    freenect_context* get() {
      return ctx;
    }
  };

  /// just copied form libfreenect.hpp
  class ICLFreenectTiltState {
    friend class ICLFreenect;
    ICLFreenectTiltState(freenect_raw_tilt_state *_state):
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



  

  

  /*---------------------------------------
    ICLFreenect Wrapper
    ---------------------------------------*/
  
  struct ICLFreenect {

    struct ICLFreenectDevice {
      freenect_device *device;
      freenect_frame_mode color_frame_mode,depth_frame_mode;
      int numColorUsers;
      int numDepthUsers;
      Mutex colorMutex, depthMutex;
      Img8u colorImage, colorImageOut;
      Img32f depthImage, depthImageOut;  
      Time lastColorTime, lastDepthTime;
      
      void depth_cb(void *data, uint32_t timestamp) {
        Mutex::Locker lock(depthMutex);
        depthImage.setTime(Time::now());
        std::copy((const icl16s*)data, (const icl16s*)data+depth_frame_mode.width*depth_frame_mode.height, depthImage.begin(0));
      }
      
      void color_cb(void *data, uint32_t timestamp) {
        Mutex::Locker lock(colorMutex);
        colorImage.setTime(Time::now());
        interleavedToPlanar((const icl8u*)data, &colorImage);
        //std::copy((const icl8u*)data, (const icl8u*)data+color_frame_mode.bytes, colorImage.begin(0));
      }
      
      const Img32f &getLastDepthImage(bool avoidDoubleFrames) {
        Mutex::Locker lock(depthMutex);
        if(avoidDoubleFrames && ( lastDepthTime != 0 && depthImage.getTime() != 0 )) {
          while(lastDepthTime == depthImage.getTime() ) {
            depthMutex.unlock();
            Thread::msleep(1);
            depthMutex.lock();
          }
        }
        lastDepthTime = depthImage.getTime();
        depthImage.deepCopy(&depthImageOut);
        return depthImageOut;
      }
      
      const Img8u &getLastColorImage(bool avoidDoubleFrames) {
        Mutex::Locker lock(colorMutex);
        if( avoidDoubleFrames && ( lastColorTime != 0 && colorImage.getTime() != 0 )) {
          while(lastColorTime == colorImage.getTime() ){
            colorMutex.unlock();
            Thread::msleep(1);
            colorMutex.lock();
          }
        }
        lastColorTime = colorImage.getTime();
        colorImage.deepCopy(&colorImageOut);
        return colorImageOut;
      }
    };
    
    KinectGrabber::Mode mode;
    
    ICLFreenectDevice *used;
    int index;
    static std::map<int,ICLFreenectDevice*> devices;
    
    ICLFreenect(ICLFreenectContext &ctx, int _index, KinectGrabber::Format format, KinectGrabber::Resolution resolution )
      : index(_index)
    {
      std::map<int,ICLFreenectDevice*>::iterator it = devices.find(index);

      if( it == devices.end() ) {
        devices[index] = used = new ICLFreenectDevice;
        
        if( freenect_open_device(ctx.get(), &used->device, index) < 0 ) {
          throw ICLException("ICLFreenect:: unable to open kinect device for device " + str(index));
        }
        used->numColorUsers = used->numDepthUsers = 0;
        freenect_set_user(used->device, used);

        freenect_set_video_callback(used->device,freenect_video_callback);
        freenect_set_depth_callback(used->device,freenect_depth_callback);
      } else {
        used = it->second;
      }

      mode = KinectGrabber::Mode(format,resolution);

      if( isGrabbingColor(mode.first) && used->numColorUsers <= 1 ) {
        used->color_frame_mode = findFreenectFrameMode(mode.first, mode.second);
        freenect_set_video_mode(used->device, used->color_frame_mode);
        
        switch(mode.first) {
          case KinectGrabber::GRAB_RGB_IMAGE:
            used->colorImage = Img8u(Size(used->color_frame_mode.width,used->color_frame_mode.height),formatRGB);
            break;
          case KinectGrabber::GRAB_BAYER_IMAGE:
            used->colorImage = Img8u(Size(used->color_frame_mode.width,used->color_frame_mode.height),formatMatrix);
            break;
          case KinectGrabber::GRAB_YUV_IMAGE:
            used->colorImage = Img8u(Size(used->color_frame_mode.width,used->color_frame_mode.height),formatYUV);
            break;
          case KinectGrabber::GRAB_IR_IMAGE:
            used->colorImage = Img8u(Size(used->color_frame_mode.width,used->color_frame_mode.height),1,formatMatrix);
            break;
          default:
            throw ICLException("ICLFreenect:: wrong color mode?");
        }
        
        if(freenect_start_video(used->device) < 0) {
          throw ICLException("ICLFreenect:: unable to start video for device " + str(index));
        }

        used->numColorUsers++;
      } else if( isGrabbingDepth(mode.first) && used->numDepthUsers <= 1 ) {
        used->depth_frame_mode = findFreenectFrameMode(mode.first, mode.second);
        freenect_set_depth_mode(used->device, used->depth_frame_mode);

        used->depthImage = Img32f(Size(used->depth_frame_mode.width,used->depth_frame_mode.height),formatMatrix);

        if(freenect_start_depth(used->device) < 0) {
          throw ICLException("ICLFreenect:: unable to start depth for device " + str(index));
        }

        used->numDepthUsers++;
      } else {
        throw ICLException("ICLFreenect:: too many users for device " + str(index));
      }
    }
             
    ~ICLFreenect() {
      if( isGrabbingColor() ) {
        used->numColorUsers--;
        if(!used->numColorUsers) {
          if(freenect_stop_video(used->device) < 0) {
            throw ICLException("ICLFreenect:: unable to stop color for device "+ str(index));
          }
        }
      } else {
        used->numDepthUsers--;
        if(!used->numDepthUsers) {
          if(freenect_stop_depth(used->device) < 0) {
            throw ICLException("ICLFreenect:: unable to stop depth for device "+ str(index));
          }
        }
      }

      if(!used->numColorUsers && !used->numDepthUsers) {
        if(freenect_close_device(used->device) < 0) {
          throw ICLException("ICLFreenect:: unable to close device "+ str(index));
        }
        devices.erase(devices.find(index));
        delete used;
      }
    }

    static void freenect_depth_callback(freenect_device *dev, void *depth, uint32_t timestamp) {
      static_cast<ICLFreenect::ICLFreenectDevice*>(freenect_get_user(dev))->depth_cb(depth,timestamp);
    }
    static void freenect_video_callback(freenect_device *dev, void *video, uint32_t timestamp) {
      static_cast<ICLFreenect::ICLFreenectDevice*>(freenect_get_user(dev))->color_cb(video,timestamp);
    }

    void setTiltDegrees(double angle) {
      if(freenect_set_tilt_degs(used->device, angle) < 0){
        throw ICLException("Cannot set angle in degrees");
      }
    }
      
    void setLed(freenect_led_options option) {
      if(freenect_set_led(used->device, option) < 0){
        throw ICLException("Cannot set led");
      }
    }
      
    void updateState() {
      if (freenect_update_tilt_state(used->device) < 0){
        throw ICLException("Cannot update device state");
      }
    }
      
    ICLFreenectTiltState getState() const {
      return ICLFreenectTiltState(freenect_get_tilt_state(used->device));
    }

    bool isGrabbingColor(int format = -1) const {
      if(format < 0) {
        format = (int)mode.first;
      }
      
      switch((KinectGrabber::Format)format) {
        case KinectGrabber::GRAB_RGB_IMAGE:
        case KinectGrabber::GRAB_BAYER_IMAGE:
        case KinectGrabber::GRAB_IR_IMAGE:
        case KinectGrabber::GRAB_YUV_IMAGE:
          return true;
        case KinectGrabber::GRAB_DEPTH_IMAGE:
        case KinectGrabber::GRAB_DEPTH_MM_IMAGE:
        case KinectGrabber::GRAB_DEPTH_CM_IMAGE:
        case KinectGrabber::GRAB_DEPTH_M_IMAGE:
        default:
          return false;
      }
    }

    bool isGrabbingDepth(int format = -1) const {
      return( ! isGrabbingColor(format) );
    }

    std::vector<std::pair<KinectGrabber::Resolution,Size> > getResolutions() {
      std::vector<std::pair<KinectGrabber::Resolution,Size> > sizes;
      freenect_frame_mode fm;
      fm = findFreenectFrameMode(mode.first, mode.second);
      sizes.push_back(std::pair<KinectGrabber::Resolution,Size>(mode.second,Size(fm.width,fm.height)));

      for( int kr = KinectGrabber::RESOLUTION_LOW; kr <= KinectGrabber::RESOLUTION_HIGH; kr++ ) {
        fm = findFreenectFrameMode(mode.first, (KinectGrabber::Resolution)kr, false);
        if( fm.is_valid && kr != mode.second ) {
          sizes.push_back(std::pair<KinectGrabber::Resolution,Size>((KinectGrabber::Resolution)kr,Size(fm.width,fm.height)));
        }
      }

      return sizes;
    }

    freenect_frame_mode findFreenectFrameMode(KinectGrabber::Format f, KinectGrabber::Resolution r, bool validate = true) {
      freenect_frame_mode fm;
      
      if( isGrabbingColor() ) {
        fm = freenect_find_video_mode( findFreenectResolution(r), findFreenectVideoFormat(f) );
      } else {
        fm = freenect_find_depth_mode( findFreenectResolution(r), findFreenectDepthFormat(f) );
      }
      
      if( ! fm.is_valid && validate ) {
        throw ICLException("ICLFreenect:: invalid mode");
      }

      return fm;
    }
    
    freenect_video_format findFreenectVideoFormat(KinectGrabber::Format f) {
      switch(f) {
        case KinectGrabber::GRAB_RGB_IMAGE:
          return FREENECT_VIDEO_RGB;
          break;
        case KinectGrabber::GRAB_BAYER_IMAGE:
          return FREENECT_VIDEO_BAYER;
          break;
        case KinectGrabber::GRAB_YUV_IMAGE:
          return FREENECT_VIDEO_YUV_RGB;
          break;
        case KinectGrabber::GRAB_IR_IMAGE:
          return FREENECT_VIDEO_IR_8BIT;
          break;
        default:
          throw ICLException("findFreenectVideoFormat:: invalid format");
          return FREENECT_VIDEO_IR_8BIT;
          break;
      };
    }

    freenect_depth_format findFreenectDepthFormat(KinectGrabber::Format f) {
      switch(f) {
        case KinectGrabber::GRAB_DEPTH_IMAGE:
        case KinectGrabber::GRAB_DEPTH_MM_IMAGE:
        case KinectGrabber::GRAB_DEPTH_CM_IMAGE:
        case KinectGrabber::GRAB_DEPTH_M_IMAGE:
          return FREENECT_DEPTH_11BIT;
          break;
        default:
          throw ICLException("findFreenectDetphFormat:: invalid format");
          return FREENECT_DEPTH_11BIT;
      }
    }

    freenect_resolution findFreenectResolution(KinectGrabber::Resolution r) {
      freenect_resolution freenect_resolution_to_icl[] = {
        FREENECT_RESOLUTION_LOW,
        FREENECT_RESOLUTION_MEDIUM,
        FREENECT_RESOLUTION_HIGH
      };
      return( freenect_resolution_to_icl[r] );
    }
  };




  inline void depth_to_distance_mm_stephane(float &f) {
    int i = (int)f;
    
    const float k1 = 1.1863;
    const float k2 = 2842.5;
    const float k3 = 0.1236;
    
    f = 1000.0f * k3 * tanf(i/k2 + k1);
  }

  inline void depth_to_distance_cm_stephane(float &f) {
    int i = (int)f;
    
    const float k1 = 1.1863;
    const float k2 = 2842.5;
    const float k3 = 0.1236;
    
    f = 100.0f * k3 * tanf(i/k2 + k1);
  }

  inline void depth_to_distance_m_stephane(float &f) {
    int i = (int)f;
    
    const float k1 = 1.1863;
    const float k2 = 2842.5;
    const float k3 = 0.1236;
    
    f = k3 * tanf(i/k2 + k1);
  }

  inline void depth_to_distance_mm(float &f) {
    if( (int)f < 2047 ) {
      f = 1000.0f / (f * -0.0030711016 + 3.3309495161);
    } else {
      f = 0.0f;
    }
  }

  inline void depth_to_distance_cm(float &f) {
    if( (int)f < 2047 ) {
      f = 100.0f / (f * -0.0030711016 + 3.3309495161);
    } else {
      f = 0.0f;
    }
  }

  inline void depth_to_distance_m(float &f) {
    if( (int)f < 2047 ) {
      f = 1.0f / (f * -0.0030711016 + 3.3309495161);
    } else {
      f = 0.0f;
    }
  }

  /*----------------------------------------
    KinectGrabber
    ----------------------------------------*/

  struct KinectGrabber::Impl {
    static SmartPtr<ICLFreenectContext> context;
    SmartPtr<ICLFreenect> freenect;
    int ledColor;
    float desiredTiltDegrees;
    bool avoidDoubleFrames;
    Mutex mutex;
    bool createdContextHere;
    std::string approximation;
    
    Impl(KinectGrabber::Format format, int index)
      : createdContextHere(false),approximation("ROS")
    {
      Mutex::Locker lock(mutex);
      if(!context) {
        createdContextHere = true;
        context = SmartPtr<ICLFreenectContext>(new ICLFreenectContext);
      }
      freenect = SmartPtr<ICLFreenect>(new ICLFreenect(*context,index,format,RESOLUTION_MEDIUM));
      if(createdContextHere) {
        context->start();
      }
    }

    void switchMode(KinectGrabber::Mode m) {
      // HACK: work around CamCfgWidget limitation, it does not update the sizes in the gui so user
      // may select an invalid mode (GRAB_YUV with RESOLUTION_HIGH in this case)
      if( m.first == KinectGrabber::GRAB_YUV_IMAGE && m.second == KinectGrabber::RESOLUTION_HIGH ) {
        m.second = RESOLUTION_MEDIUM;
      }
      
      if(freenect->mode.first != m.first || freenect->mode.second != m.second ) {
        int index = freenect->index;
        freenect = 0;
        ICLFreenect *newFreenect = new ICLFreenect(*context,index,m.first,m.second);
        freenect = SmartPtr<ICLFreenect>(newFreenect);
      }
    }
    
    inline const Img32f &getLastDepthImage() {
      return freenect->used->getLastDepthImage(avoidDoubleFrames);
    }
    inline const Img8u &getLastColorImage() {
      return freenect->used->getLastColorImage(avoidDoubleFrames);
    }
  };
  
    
  std::map<int,ICLFreenect::ICLFreenectDevice*> ICLFreenect::devices; 
  
  SmartPtr<ICLFreenectContext> KinectGrabber::Impl::context;



  KinectGrabber::KinectGrabber(KinectGrabber::Format format, int deviceID) throw (ICLException)
    : m_impl(new Impl(format,deviceID))
  {
    m_impl->ledColor = 0;
    m_impl->desiredTiltDegrees = 0;
    m_impl->avoidDoubleFrames  = true;
  }
  
  KinectGrabber::~KinectGrabber() {
    delete m_impl;
  }
  
  const ImgBase* KinectGrabber::acquireImage() {
    Mutex::Locker lock(m_impl->mutex);
    if( m_impl->freenect->isGrabbingColor() ) {
      return &m_impl->getLastColorImage();
    } else {
      switch(m_impl->freenect->mode.first) {
        case GRAB_DEPTH_MM_IMAGE: {
          Img32f* img = new Img32f(m_impl->getLastDepthImage());
          if( m_impl->approximation == "Stephane" ) {
            return &img->forEach_C(depth_to_distance_mm_stephane,0);
          } else {
            return &img->forEach_C(depth_to_distance_mm,0);
          }
        }
        case GRAB_DEPTH_CM_IMAGE: {
          Img32f* img = new Img32f(m_impl->getLastDepthImage());
          if( m_impl->approximation == "Stephane" ) {
            return &img->forEach_C(depth_to_distance_cm_stephane,0);
          } else {
            return &img->forEach_C(depth_to_distance_cm,0);
          }
        }
        case GRAB_DEPTH_M_IMAGE: {
          Img32f* img = new Img32f(m_impl->getLastDepthImage());
          if( m_impl->approximation == "Stephane") {
            return &img->forEach_C(depth_to_distance_m_stephane,0);
          } else {
            return &img->forEach_C(depth_to_distance_m,0);
          }
        }
        case GRAB_DEPTH_IMAGE:
        default:
          return &m_impl->getLastDepthImage();
      }
    }
  }
  
  /// get type of property 
  std::string KinectGrabber::getType(const std::string &name) {
    Mutex::Locker lock(m_impl->mutex);
    if(name == "format" || name == "size" || name == "LED" || name == "Distance-Approximation") return "menu";
    else if(name == "Desired-Tilt-Angle") {
      return "range";
    } else if(name =="Current-Tilt-Angle" || "Accelerometers") {
      return "info";
    } else {
      return "undefined";
    }
  }
  
  /// get information of a properties valid values
  std::string KinectGrabber::getInfo(const std::string &name){
    Mutex::Locker lock(m_impl->mutex);
    if(name == "format") {
      if( m_impl->freenect->isGrabbingColor() ) {
        return "{\"RGB\",\"Bayer\",\"IR\",\"YUV\"}";
      } else {
          return "{\"Depth (raw)\",\"Depth (normalized)\",\"Depth (mm)\",\"Depth (cm)\",\"Depth (m)\"}";
      }
    } else if(name == "size") {
      std::vector<std::pair<Resolution,Size> > resolutions = m_impl->freenect->getResolutions();
      std::stringstream ret;

      ret << "{";
      for( std::vector<std::pair<Resolution,Size> >::iterator p = resolutions.begin(); p != resolutions.end(); ++p ) {
        ret << "\"" << p->second.width << "x" << p->second.height << "\"";
        if( p+1 == resolutions.end() ) {
          ret << "}";
        } else {
          ret << ",";
        }
      }
      return ret.str();
    } else if(name == "LED") {
      return "{\"off\",\"green\",\"red\",\"yellow\",\"blink yellow\",\"blink green\",\"blink red/yellow\"}";
    } else if(name == "Distance-Approximation") {
      return "{\"ROS\",\"Stephane\"}";
    } else if(name == "Desired-Tilt-Angle") {
      return "[-35,25]";
    } else if(name == "Current-Tilt-Angle" || name == "Accelerometers") {
      return "undefined for type info!";
    } else {
      return "undefined for type info!";
    }
  }
  
  /// returns the current value of a property or a parameter
  std::string KinectGrabber::getValue(const std::string &name) {
    Mutex::Locker lock(m_impl->mutex);
    if(name == "format") {
      switch(m_impl->freenect->mode.first) {
        case GRAB_RGB_IMAGE:
          return "RGB";
        case GRAB_BAYER_IMAGE:
          return "Bayer";
        case GRAB_IR_IMAGE:
          return "IR";
        case GRAB_YUV_IMAGE:
          return "YUV";
        case GRAB_DEPTH_MM_IMAGE:
          return "Depth (mm)";
        case GRAB_DEPTH_CM_IMAGE:
          return "Depth (cm)";
        case GRAB_DEPTH_M_IMAGE:
          return "Depth (m)";
        case GRAB_DEPTH_IMAGE:
        default:
          return "Depth (raw)";
      }
      return "undefined";
    } else if(name == "size") {
      std::stringstream ret;
      if( m_impl->freenect->isGrabbingColor() ) {
        ret << "\"" << m_impl->freenect->used->color_frame_mode.width << "x" << m_impl->freenect->used->color_frame_mode.height << "\"";
      } else {
        ret << "\"" << m_impl->freenect->used->depth_frame_mode.width << "x" << m_impl->freenect->used->depth_frame_mode.height << "\"";
      }
      return ret.str();
    } else if(name == "LED") {
      return str(m_impl->ledColor);
    } else if(name == "Distance-Approximation") {
      return m_impl->approximation;
    } else if(name == "Desired-Tilt-Angle") {
      return str(m_impl->desiredTiltDegrees);
    } else if(name == "Current-Tilt-Angle") {
      try{
        m_impl->freenect->updateState();
      } catch(...) {}
      double degs = m_impl->freenect->getState().getTiltDegs();
      if(degs == -64) return "moving";
      return str(degs);
    } else if( name == "Accelerometers") {
      try {
        m_impl->freenect->updateState();
      } catch(...) {}
      double a[3]={0,0,0};
      m_impl->freenect->getState().getAccelerometers(a,a+1,a+2);
      return str(a[0]) + "," + str(a[1]) + "," + str(a[2]);
    } else {
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
    if(name == "format") {
      Mode p = m_impl->freenect->mode;
      if(value == "RGB") {
        p.first = GRAB_RGB_IMAGE;
      } else if(value == "Bayer") {
        p.first = GRAB_BAYER_IMAGE;
      } else if(value == "IR") {
        p.first = GRAB_IR_IMAGE;
      } else if(value == "YUV") {
        p.first = GRAB_YUV_IMAGE;
      } else if(value == "Depth (mm)") {
        p.first = GRAB_DEPTH_MM_IMAGE;
      } else if(value == "Depth (cm)") {
        p.first = GRAB_DEPTH_CM_IMAGE;
      } else if(value == "Depth (m)") {
        p.first = GRAB_DEPTH_M_IMAGE;
      } else if(value == "Depth (raw)") {
        p.first = GRAB_DEPTH_IMAGE;
      }
      m_impl->switchMode(p);
    } else if(name == "size") {
      Mode p = m_impl->freenect->mode;

      std::stringstream s(value);
      std::string identifier;
      std::getline(s,identifier,'x');

      if( identifier == "320" ) {
        p.second = RESOLUTION_LOW;
      } else if( identifier == "640" ) {
        p.second = RESOLUTION_MEDIUM;
      } else if( identifier == "1280" ) {
        p.second = RESOLUTION_HIGH;
      }

      m_impl->switchMode(p);
    } else if(name == "LED") {
      if(value == "off") {
        m_impl->freenect->setLed((freenect_led_options)0);
      } else if(value == "green") {
        m_impl->freenect->setLed((freenect_led_options)1);
      } else if(value == "red") {
        m_impl->freenect->setLed((freenect_led_options)2);
      } else if(value == "yellow") {
        m_impl->freenect->setLed((freenect_led_options)3);
      } else if(value == "blink yellow") {
        m_impl->freenect->setLed((freenect_led_options)4);
      } else if(value == "blink green") {
        m_impl->freenect->setLed((freenect_led_options)5);
      } else if(value == "blink red/yellow") {
        m_impl->freenect->setLed((freenect_led_options)6);
      } else {
        ERROR_LOG("invalid property value for property 'LED'");
      }

    } else if(name == "Distance-Approximation") {
      m_impl->approximation = value;
    } else if(name == "Desired-Tilt-Angle") {
      m_impl->freenect->setTiltDegrees(parse<double>(value));
    } else if(name == "Current-Tilt-Angle" || name == "Accelerometers") {
      ERROR_LOG("info-properties cannot be set");
    } else {
      ERROR_LOG("invalid property name '" << name << "'"); 
    }
  }
  
  /// returns a list of properties, that can be set using setProperty
  std::vector<std::string> KinectGrabber::getPropertyList(){
    Mutex::Locker lock(m_impl->mutex);
    static std::string props[] = {"format","size","LED","Distance-Approximation","Desired-Tilt-Angle","Current-Tilt-Angle","Accelerometers"};
    return std::vector<std::string>(props,props+6);
  }

  /// returns a list of attached kinect devices
  const std::vector<GrabberDeviceDescription> &KinectGrabber::getDeviceList(bool rescan) {
    //DEBUG_LOG("called with rescan = " << (rescan?"true":"false"));
    static std::vector<GrabberDeviceDescription> deviceList;
    if(rescan){
      deviceList.clear();
      for(int i=0;i<8;++i){
        try{
          KinectGrabber g(GRAB_RGB_IMAGE,i);
          deviceList.push_back(GrabberDeviceDescription("kinectc",str(i),"Kinect Color Camera (ID "+str(i)+")"));
          deviceList.push_back(GrabberDeviceDescription("kinectd",str(i),"Kinect Depth Camera (ID "+str(i)+")"));
        }catch(ICLException &e){
          (void)e;
          //SHOW(e.what());
          break;
        }
      }
    }
    return deviceList;
  }
}


#endif
