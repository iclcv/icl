#include "iclDCDevice.h"
#include <iclStrTok.h>
#include <iclMacros.h>
#include <iclDC.h>

using namespace std;
using namespace icl::dc;
namespace icl{

  const DCDevice DCDevice::null = DCDevice(0);
  
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
    if(isNull()) return "null";  
    return m_poCam->vendor;
  }
  
  string DCDevice::getModelID() const{
    if(isNull()) return "null";  
    return m_poCam->model;
  }

  icl32s DCDevice::getPort() const{
    if(isNull()) return -1;
    return m_poCam->port;
  }
  
  icl16s DCDevice::getNode() const{
    if(isNull()) return (icl16s)-1; /// ??
    return m_poCam->node;
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
      dc1394_print_camera_info(m_poCam);
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
  bool DCDevice::supports(format fmt) const{
    // {{{ open

    if(isNull()) return false;
    if( is_firefly_mono(m_poCam) ){
      return fmt == formatGray || 
             fmt == formatMatrix;
    }else if( is_firefly_color(m_poCam) ){
      return fmt == formatRGB || 
             fmt == formatMatrix;
    }else if( string(m_poCam->model) == "DFW-VL500 2.30"){
      return fmt == formatRGB || fmt == formatMatrix;
    }else if( string(m_poCam->vendor) == "Apple Computer, Inc."){
      return fmt == formatRGB || fmt == formatMatrix;
    }else{
      ERROR_LOG("unsupported camera: \"" << m_poCam->model << "\"");
      return false;
    }
  }

  // }}}
  bool DCDevice::supports(const Size &size) const{
    // {{{ open

    if(isNull()) return false;
    if( is_firefly_mono(m_poCam) ){
      return size == Size(640,480);
    }else if( is_firefly_color(m_poCam) ){
      return size == Size(640,480) || 
             size == Size(320,240);  // in this case by BAYER_DOWNSAMPLE is used
    }else if(string(m_poCam->vendor) == "Apple Computer, Inc."){
      return size == Size(640,480) || 
	     size == Size(320,240);
    }else{
      ERROR_LOG("unsupported camera!");
      return false;
    }
  }

  // }}}
  bool DCDevice::supports(const DCDevice::Mode &mode) const{
    // {{{ open

    if(isNull()) return false;
    return mode.supportedBy(m_poCam);
  }

  // }}}

  bool DCDevice::needsBayerDecoding() const{
    // {{{ open

    if(isNull()) return false; 
    if( is_firefly_mono(m_poCam) ){
      return false;
    }else if( is_firefly_color(m_poCam) ){
      return true;
    }else if( string(m_poCam->model) == "DFW-VL500 2.30"){
      return false;
    }else if( string(m_poCam->vendor) == "Apple Computer, Inc."){
      return false;
    }else{
      ERROR_LOG("unsupported camera: \""<< m_poCam->model << "\"");
      return false;
    }
  }

  // }}}
  dc1394color_filter_t DCDevice::getBayerFilterLayout() const{
    // {{{ open

    if(isNull()) return (dc1394color_filter_t)0;
    if( is_firefly_mono(m_poCam) ){
      return (dc1394color_filter_t)0;
    }else if( is_firefly_color(m_poCam) ){
      return DC1394_COLOR_FILTER_GBRG;
    }else{
      return (dc1394color_filter_t)0;
    }
  }

  // }}}


  bool DCDevice::isFeatureAvailable(const string &s) const { 
    // {{{ open

    if(isNull()) return false; 
    dc1394bool_t isAvailable=DC1394_FALSE;
    dc1394_feature_is_present(m_poCam,feature_from_string(s),&isAvailable);
    return isAvailable == DC1394_TRUE ? true : false;
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
    return supported;
  }

  // }}}
  
  string DCDevice::getFeatureType(const string &feature) const{
    // {{{ open
    if(isNull()) return "";
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

    
}
