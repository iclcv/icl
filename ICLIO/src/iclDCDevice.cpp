#include "iclDCDevice.h"
#include <iclStrTok.h>
#include <iclMacros.h>
#include <iclDC.h>
#include <stdio.h>
#include <iclThread.h>

#include <algorithm>

using namespace std;
using namespace icl::dc;
namespace icl{

  /** Have a NEW CAMERA ???

      - add a new CameryTypeID to the DCDevice::enum
 
      - edit the function trailed wiht token **NEW-CAM**
        - bool DCDevice::supports(format) const
        - bool DCDevice::supports(const Size &) const
        - bool DCDevice::needsBayerDecoding() const 
        - dc1394color_filter_t DCDevice::getBayerFilterLayout() const
      
  **/
  
  namespace{
    bool is_trigger_name(const std::string &n, bool withPolarity=false){
      if(n.length() < 8 || n.substr(0,8) != "trigger-") return false;
      const std::string s = n.substr(8);
      return s=="power" || s=="mode" || s=="source" || s=="from-software" || (withPolarity && s=="polarity");
    }
    
    bool has_trigger_polarity(dc1394camera_t *cam){
      dc1394bool_t hasPolarity = DC1394_FALSE;
      dc1394_external_trigger_has_polarity(cam,&hasPolarity);
      return hasPolarity;
    }
    std::string switch_to_string(dc1394switch_t s){
      return s==DC1394_OFF ? "off" :
             s==DC1394_ON  ? "on" : "";
    }
    
  }
  
  const DCDevice DCDevice::null = DCDevice(0);

  // **NEW-CAM** (optional)
  std::string DCDevice::translate(DCDevice::CameraTypeID id){
    // {{{ open

    switch(id){
#define TRANSLATE(X) case X: return #X
      TRANSLATE(pointGreyFire_FlyMVMono);
      TRANSLATE(pointGreyFire_FlyMVColor);
      TRANSLATE(sony_DFW_VL500_2_30);
      TRANSLATE(apple_ISight);
      TRANSLATE(fireI_1_2);
      TRANSLATE(imagingSource_DFx_21BF04);
#undef TRANSLATE
      default: return "unknownCameraType";
    }    
  }

  // }}}

  // **NEW-CAM** (optional)
  DCDevice::CameraTypeID DCDevice::translate(const std::string &name){
    // {{{ open

    if(name == "pointGreyFire_FlyMVMono" ) return pointGreyFire_FlyMVMono;
#define TRANSLATE(X) else if( name == #X ) return X
    TRANSLATE(pointGreyFire_FlyMVColor);
    TRANSLATE(sony_DFW_VL500_2_30);
    TRANSLATE(apple_ISight);
    TRANSLATE(fireI_1_2);
    TRANSLATE(imagingSource_DFx_21BF04);
#undef TRANSLATE 
    else return unknownCameraType;
  }

  // }}}
  
  // **NEW-CAM** 
  DCDevice::CameraTypeID DCDevice::estimateCameraType(dc1394camera_t *cam){
    // {{{ open
    if(!cam){
      return unknownCameraType;
    }else if( is_firefly_mono(cam) ){
      return pointGreyFire_FlyMVMono;
    }else if( is_firefly_color(cam) ){
      return pointGreyFire_FlyMVColor;
    }else if( string(cam->model) == "DFW-VL500 2.30"){
      return sony_DFW_VL500_2_30;
    }else if( string(cam->vendor) == "Apple Computer, Inc."){
      return apple_ISight;
    }else if( string(cam->model) == "Fire-i 1.2"){
      return fireI_1_2;
    }else if( string(cam->model) == "DFx 21BF04"){
      return imagingSource_DFx_21BF04;
    }else{
      ERROR_LOG("unsupported camera: \"" << cam->model << "\"");
      return unknownCameraType;
    }  
  }

  // }}}

  void DCDevice::dc1394_reset_bus(bool verbose){
    dc1394_t * d;
    dc1394camera_list_t * list;
    dc1394camera_t *camera;
    dc1394error_t err;
    
    d = dc1394_new ();
    err=dc1394_camera_enumerate (d, &list);
    DC1394_ERR(err,"Failed to enumerate cameras");
    
    if (list->num == 0) {
      dc1394_log_error("No cameras found");
      return;
    }
    
    camera = dc1394_camera_new (d, list->ids[0].guid);
    if (!camera) {
      dc1394_log_error("Failed to initialize camera with guid %llx", list->ids[0].guid);
      return;
    }
    dc1394_camera_free_list (list);
    
    if(verbose){
      printf("Using camera with GUID %llx\n", camera->guid);
      printf ("Reseting bus...\n");
    }
    
    ::dc1394_reset_bus (camera);
    
    dc1394_camera_free (camera);
    dc1394_free (d);
    
    Thread::msleep(1000);
  }

  
  vector<DCDevice::Mode> DCDevice::getModes() const{
    // {{{ open

    vector<DCDevice::Mode> v;
    ICLASSERT_RETURN_VAL( !isNull(), v);
                 
    dc1394video_modes_t modeList;
    dc1394_video_get_supported_modes(m_poCam,&modeList);
    for(unsigned int i=0;i<modeList.num;i++){
      if(modeList.modes[i] < DC1394_VIDEO_MODE_FORMAT7_MIN){
        dc1394framerates_t framerateList;
        dc1394_video_get_supported_framerates(m_poCam,modeList.modes[i],&framerateList);
        for(unsigned int j=0;j<framerateList.num;j++){
          v.push_back(DCDevice::Mode(modeList.modes[i],framerateList.framerates[j]));
        }
      }
    }
    return v;
  }

  // }}}
  string DCDevice::getVendorID() const{
    // {{{ open

    if(isNull()) return "null";  
    return m_poCam->vendor;
  }

  // }}}
  string DCDevice::getModelID() const{
    // {{{ open

    if(isNull()) return "null";  
    return m_poCam->model;
  }

  // }}}

  uint64_t DCDevice::getGUID() const{
    if(isNull()) return -1;
    return m_poCam->guid;
  }

  
  icl32s DCDevice::getUnit() const{
    if(isNull()) return -1;
    return m_poCam->unit;
  }

  icl32s DCDevice::getUnitSpecID() const{
    if(isNull()) return -1;
    return m_poCam->unit_spec_ID;
  }

  void DCDevice::setMode(const Mode &mode){
    // {{{ open

    ICLASSERT_RETURN( !isNull() );
    dc1394error_t err = dc1394_video_set_mode(m_poCam,mode.videomode);
    ICLASSERT_RETURN( err == DC1394_SUCCESS );
    err = dc1394_video_set_framerate(m_poCam,mode.framerate);
    ICLASSERT_RETURN( err == DC1394_SUCCESS );
  }

  // }}}
  void DCDevice::show(const string &title) const{
    // {{{ open

    printf("DCDevice: %s \n",title.c_str());
    if(isNull()){
      printf("null! \n");
    }else{
      dc1394_camera_print_info(m_poCam,stdout);
      printf("-----------------------------\n");
      printf("supported modes: \n");
      vector<DCDevice::Mode> v = getModes();
      for(unsigned int i=0;i<v.size();i++){
        printf(" %2d: %s \n",i,v[i].toString().c_str());
      }
      printf("-----------------------------\n");
    }
  }

  // }}}
  bool DCDevice::supports(const DCDevice::Mode &mode) const{
    // {{{ open

    if(isNull()) return false;
    return mode.supportedBy(m_poCam);
  }

  // }}}
 
  
  // **NEW-CAM** 
  bool DCDevice::supports(format fmt) const{
    // {{{ open
    
    if(isNull()) return false;
    switch(m_eCameraTypeID){
      case pointGreyFire_FlyMVMono:
        return fmt == formatGray || fmt == formatMatrix;
      case pointGreyFire_FlyMVColor:
      case sony_DFW_VL500_2_30:
      case apple_ISight:
      case fireI_1_2:
      case imagingSource_DFx_21BF04:
        return fmt == formatRGB ||  fmt == formatMatrix;
      case unknownCameraType:
      default:
        return false;
    }
  }

  // }}}
  
  // **NEW-CAM** 
  bool DCDevice::supports(const Size &size) const{
    // {{{ open

    if(isNull()) return false;
    static const Size s64(640,480);
    static const Size s32(640,480);
    switch(m_eCameraTypeID){
      case pointGreyFire_FlyMVMono: 
      case imagingSource_DFx_21BF04:
        return size == s64;
      case pointGreyFire_FlyMVColor:
      case sony_DFW_VL500_2_30:
      case apple_ISight:
      case fireI_1_2:
        return size == s64 || size == s32;
      case unknownCameraType:
      default:
        return false;
    }
  }

  // }}}
  
  // **NEW-CAM** 
  bool DCDevice::needsBayerDecoding() const{
    // {{{ open

    if(isNull()) return false; 
    switch(m_eCameraTypeID){
      case pointGreyFire_FlyMVMono: 
      case apple_ISight:
      case sony_DFW_VL500_2_30:
      case fireI_1_2:
        return false;
      case pointGreyFire_FlyMVColor:
        return true;
      case imagingSource_DFx_21BF04:        
        if(getMode().videomode == DC1394_VIDEO_MODE_640x480_YUV422){
          return false;
        }else{
          return true;
        }
      case unknownCameraType:
      default:
        return false;
    }
  }

  // }}}
  
  // **NEW-CAM**   
  dc1394color_filter_t DCDevice::getBayerFilterLayout() const{
    // {{{ open
    if(isNull()) return (dc1394color_filter_t)0;
    switch(m_eCameraTypeID){
      case pointGreyFire_FlyMVColor:
      case imagingSource_DFx_21BF04:
        return DC1394_COLOR_FILTER_GBRG;
      default:
        return (dc1394color_filter_t)0;
    }
  }

  // }}}


  bool DCDevice::isFeatureAvailable(const string &s) const { 
    // {{{ open

    if(isNull()) return false; 
    if(is_trigger_name(s)){
      return true;
    }else if(has_trigger_polarity(getCam())){
      return true;
    }else{
      dc1394bool_t isAvailable=DC1394_FALSE;
      dc1394_feature_is_present(m_poCam,feature_from_string(s),&isAvailable);
      return isAvailable == DC1394_TRUE ? true : false;
    }
  }

  // }}}
  vector<string> DCDevice::getFeatures() const{
    // {{{ open

    const vector<string> &v = getListOfAllFeatures();
    vector<string> supported;
    for(unsigned int i=0;i<v.size();++i){
      if(isFeatureAvailable(v[i])){
        string featureType = getFeatureType(v[i]);
        if(featureType == "range" || featureType == "menu"){
          supported.push_back(v[i]);
        }
      }
    }
    
    if(has_trigger_polarity(getCam())){
      supported.push_back("trigger-polarity");
    }
    supported.push_back("trigger-power");
    supported.push_back("trigger-mode");
    supported.push_back("trigger-source");
    supported.push_back("trigger-from-software");

    return supported;
  }

  // }}}
  
  string DCDevice::getFeatureType(const string &feature) const{
    // {{{ open
    if(isNull()) return "";
    if(is_trigger_name(feature,true)){
      return "menu";
    }
    
    dc1394feature_t f = feature_from_string(feature);
    dc1394bool_t hasAbsoluteControl;
    dc1394_feature_has_absolute_control(m_poCam,f,&hasAbsoluteControl);
    if(hasAbsoluteControl){
      dc1394switch_t absoluteControlModeSet;
      dc1394_feature_get_absolute_control(m_poCam,f,&absoluteControlModeSet);
      if(absoluteControlModeSet == DC1394_OFF){
        dc1394_feature_set_absolute_control(m_poCam,f,DC1394_ON);
      }
      return "range";
    }
    
    dc1394bool_t isSwitchable;
    dc1394_feature_is_switchable(m_poCam,f,&isSwitchable);
    if(isSwitchable){
      return "menu";
    }
    return "";
  }

  // }}}
  
  string DCDevice::getFeatureInfo(const string &feature) const{
    // {{{ open
    if(isNull()) return "";
    if(is_trigger_name(feature,true)){
      const std::string f = feature.substr(8);
      if(f=="polarity"){
        return "{\"low\",\"high\"}";
      }else if(f=="power"){
        return "{\"on\",\"off\"}";
      }else if(f=="mode"){
        return "{\"0\",\"1\",\"2\",\"3\",\"4\",\"5\",\"14\",\"15\"}";
      }else if(f=="source"){
        return "{\"0\",\"1\",\"2\",\"3\"}";
      }else if(f=="from-software"){
        return "{\"on\",\"off\"}";
      }else{
        return "";
      }
    }
    dc1394feature_t f = feature_from_string(feature);
    dc1394bool_t hasAbsoluteControl;
    dc1394_feature_has_absolute_control(m_poCam,f,&hasAbsoluteControl);
    if(hasAbsoluteControl){
      dc1394switch_t absoluteControlModeSet;
      dc1394_feature_get_absolute_control(m_poCam,f,&absoluteControlModeSet);
      if(absoluteControlModeSet == DC1394_OFF){
        dc1394_feature_set_absolute_control(m_poCam,f,DC1394_ON);
      }
      float minVal,maxVal;
      dc1394_feature_get_absolute_boundaries(m_poCam,f,&minVal,&maxVal);
      char buf[100];
      sprintf(buf,"[%f,%f]:0",minVal,maxVal);
      return buf;
    }
    dc1394bool_t isSwitchable;
    dc1394_feature_is_switchable(m_poCam,f,&isSwitchable);
    if(isSwitchable){
      return "{\"on\",\"off\"}";
    } 
    return "";
  }

  // }}}

  string DCDevice::getFeatureValue(const string &feature) const{
    // {{{ open
    if(isNull()) return "";
    if(is_trigger_name(feature,true)){
      const std::string f = feature.substr(8);
      if(f=="polarity"){
        dc1394trigger_polarity_t t;
        dc1394_external_trigger_get_polarity(getCam(),&t);
        return t==DC1394_TRIGGER_ACTIVE_LOW  ? "low" :
               t==DC1394_TRIGGER_ACTIVE_HIGH ? "high" : "";
      }else if(f=="power"){
        dc1394switch_t s;
        dc1394_external_trigger_get_power(getCam(),&s);
        return switch_to_string(s);
      }else if(f=="mode"){
        dc1394trigger_mode_t m;
        dc1394_external_trigger_get_mode(getCam(),&m);
        static const char* values[] = {"0","1","2","3","4","5","14","15"};
        int idx = (int)m - (int)DC1394_TRIGGER_MODE_0;
        if(idx >= 0 && idx < 8){
          return values[idx];
        }else{
          return "";
        }
      }else if(f=="source"){
        dc1394trigger_source_t s;
        dc1394_external_trigger_get_source(getCam(),&s);
        static const char *values [] = {"0","1","2","3"};
        int idx = (int)s - (int)DC1394_TRIGGER_SOURCE_0;
        if(idx >=0 && idx < 4){
          return values[idx];
        }else{
          return "";
        }
      }else if(f=="from-software"){
        dc1394switch_t s;
        dc1394_software_trigger_get_power(getCam(),&s);
        return switch_to_string(s);
      }else{
        return "";
      }
    }

    dc1394feature_t f = feature_from_string(feature);
    dc1394bool_t hasAbsoluteControl;
    dc1394_feature_has_absolute_control(m_poCam,f,&hasAbsoluteControl);
    if(hasAbsoluteControl){
      dc1394switch_t absoluteControlModeSet;
      dc1394_feature_get_absolute_control(m_poCam,f,&absoluteControlModeSet);
      if(absoluteControlModeSet == DC1394_OFF){
        dc1394_feature_set_absolute_control(m_poCam,f,DC1394_ON);
      }
      float val=0;
      dc1394_feature_get_absolute_value(m_poCam,f,&val);
      char buf[20];
      sprintf(buf,"%f",val);
      return buf;
    }
    dc1394bool_t isSwitchable;
    dc1394_feature_is_switchable(m_poCam,f,&isSwitchable);
    if(isSwitchable){
      dc1394switch_t pwr = DC1394_OFF;
      dc1394_feature_get_power(m_poCam,f,&pwr);
      return pwr == DC1394_OFF ? "off" : "on";
    } 
    return "0";
  }

  // }}}
  
  void DCDevice::setFeatureValue(const string &feature, const string &value){
    // {{{ open
    if(isNull()) return;
    if(is_trigger_name(feature,true)){
      const std::string f = feature.substr(8);
      if(f=="polarity"){
        if(value == "low"){
          dc1394_external_trigger_set_polarity(getCam(),DC1394_TRIGGER_ACTIVE_LOW);
        }else if (value == "high"){
          dc1394_external_trigger_set_polarity(getCam(),DC1394_TRIGGER_ACTIVE_HIGH);
        }else{
          ERROR_LOG("invalid value for feature trigger-polarity: " << value);
        }        
      }else if(f=="power"){
        if(value == "on"){
          dc1394_external_trigger_set_power(getCam(),DC1394_ON);
        }else if(value == "off"){
          dc1394_external_trigger_set_power(getCam(),DC1394_OFF);
        }else{
          ERROR_LOG("invalid value for feature trigger-power: " << value);
        }
      }else if(f=="mode"){
        if(value == "0")dc1394_external_trigger_set_mode(getCam(),DC1394_TRIGGER_MODE_0);
        else if(value == "1")dc1394_external_trigger_set_mode(getCam(),DC1394_TRIGGER_MODE_1);
        else if(value == "2")dc1394_external_trigger_set_mode(getCam(),DC1394_TRIGGER_MODE_2);
        else if(value == "3")dc1394_external_trigger_set_mode(getCam(),DC1394_TRIGGER_MODE_3);
        else if(value == "4")dc1394_external_trigger_set_mode(getCam(),DC1394_TRIGGER_MODE_4);
        else if(value == "5")dc1394_external_trigger_set_mode(getCam(),DC1394_TRIGGER_MODE_5);
        else if(value == "14")dc1394_external_trigger_set_mode(getCam(),DC1394_TRIGGER_MODE_14);
        else if(value == "15")dc1394_external_trigger_set_mode(getCam(),DC1394_TRIGGER_MODE_15);
        else ERROR_LOG("invalid value for feature trigger-mode: " << value);
      }else if(f=="source"){
        if(value == "0") dc1394_external_trigger_set_source(getCam(),DC1394_TRIGGER_SOURCE_0);
        else if(value == "1") dc1394_external_trigger_set_source(getCam(),DC1394_TRIGGER_SOURCE_1);
        else if(value == "2") dc1394_external_trigger_set_source(getCam(),DC1394_TRIGGER_SOURCE_2);
        else if(value == "3") dc1394_external_trigger_set_source(getCam(),DC1394_TRIGGER_SOURCE_3);
        else ERROR_LOG("invalid value for feature trigger-source: " << value);
      }else if(f=="from-software"){
        if(value == "on"){
          dc1394_software_trigger_set_power(getCam(),DC1394_ON);
        }else if(value == "off"){
          dc1394_software_trigger_set_power(getCam(),DC1394_OFF);
        }else{
          ERROR_LOG("invalid value for feature trigger-from-software: " << value);
        }
      }else{
        ERROR_LOG("invalid feature name: " << feature << "(value was " << value << ")");
      }
      return;
    }

    dc1394feature_t f = feature_from_string(feature);

    dc1394bool_t hasAbsoluteControl;
    dc1394_feature_has_absolute_control(m_poCam,f,&hasAbsoluteControl);
    if(hasAbsoluteControl){
      dc1394switch_t absoluteControlModeSet;
      dc1394_feature_get_absolute_control(m_poCam,f,&absoluteControlModeSet);
      if(absoluteControlModeSet == DC1394_OFF){
        dc1394_feature_set_absolute_control(m_poCam,f,DC1394_ON);
      }
      float val = atof(value.c_str());
      dc1394_feature_set_absolute_value(m_poCam,f,val);
      return;
    }

    dc1394bool_t isSwitchable;
    dc1394_feature_is_switchable(m_poCam,f,&isSwitchable);
    if(isSwitchable){
      if(value != "on" && value != "off"){
        ERROR_LOG("feature \""<<feature<<"\" cannot be set to \""<<value<<"\", values are \"on\" and \"off\"");
        return;
      }
      dc1394switch_t pwr = value == "on" ? DC1394_OFF : DC1394_ON;
      dc1394_feature_set_power(m_poCam,f,pwr);
    } 
  }

  // }}}
  
  DCDevice::Mode::Mode(dc1394camera_t *cam){
    // {{{ open

    if(cam){
      dc1394_video_get_mode(cam,&videomode);
      dc1394_video_get_framerate(cam,&framerate);
    }else{
      framerate = (dc1394framerate_t)-1;
      videomode = (dc1394video_mode_t)-1;
    }
  }

  // }}}
  DCDevice::Mode::Mode(const string &stringRepr){
    // {{{ open

    StrTok s(stringRepr,"@");
    const vector<string> &toks = s.allTokens();
    ICLASSERT(toks.size() == 2);
    videomode = dc::videomode_from_string(toks[0]);
    framerate = dc::framerate_from_string(toks[1]);
    
  }

  // }}}
  string DCDevice::Mode::toString() const{
    // {{{ open

    return dc::to_string(videomode)+"@"+dc::to_string(framerate);
  }

  // }}}
  bool DCDevice::Mode::supportedBy(dc1394camera_t *cam) const{
    // {{{ open

    dc1394video_modes_t modeList; 
    dc1394_video_get_supported_modes(cam,&modeList);
    for(unsigned int i=0;i<modeList.num;i++){
      if(modeList.modes[i] == videomode){
        dc1394framerates_t framerateList;
        dc1394_video_get_supported_framerates(cam,modeList.modes[i],&framerateList);
        for(unsigned int j=0;j<framerateList.num;j++){
          if(framerateList.framerates[j] == framerate){
            return true;
          }
        }
      }
    }
    return false;
  }

  // }}}

  void DCDevice::setISOSpeed(int mbits){
    // {{{ open
    ICLASSERT_RETURN(!isNull());
    dc::set_iso_speed(getCam(),mbits);
  }
  // }}}
    
}
