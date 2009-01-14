#include "iclDCDevice.h"
#include <iclStrTok.h>
#include <iclMacros.h>
#include <iclDC.h>
#include <stdio.h>
#include <iclThread.h>

#include <algorithm>
#include <iclStringUtils.h>
#include <set>
#include <string>

using namespace std;
using namespace icl::dc;
namespace icl{

  /** Have a NEW CAMERA ???

      - add a new CameryTypeID to the DCDevice::enum
 
      - edit the function trailed wiht token **NEW-CAM**
        - translate(CameraTypeID)
        - translate(std::string)
        - estimateCameraID
        - bool DCDevice::needsBayerDecoding() const 
        - dc1394color_filter_t DCDevice::getBayerFilterLayout() const
      
  **/
  

  
  const DCDevice DCDevice::null = DCDevice(0);

#if 0
  // **NEW-CAM** (optional)
  std::string DCDevice::translate(DCDevice::CameraTypeID id){
    // {{{ open

    switch(id){
#define TRANSLATE(X) case X: return #X
      TRANSLATE(pointGrey_Fire_FlyMVMono);
      TRANSLATE(pointGrey_Fire_FlyMVColor);
      TRANSLATE(sony_DFW_VL500_2_30);
      TRANSLATE(apple_ISight);
      TRANSLATE(fireI_1_2);
      TRANSLATE(imagingSource_DFx_21BF04);
      TRANSLATE(pointGrey_Flea2_08S2C);
      TRANSLATE(pointGrey_Flea2_03S2M);
      TRANSLATE(pointGrey_Flea2_03S2C);
      TRANSLATE(pointGrey_Flea2G_13S2CC);
#undef TRANSLATE
      default: return "unknownCameraType";
    }    
  }

  // }}}

  // **NEW-CAM** (optional)
  DCDevice::CameraTypeID DCDevice::translate(const std::string &name){
    // {{{ open

    if(name == "pointGrey_Fire_FlyMVMono" ) return pointGrey_Fire_FlyMVMono;
#define TRANSLATE(X) else if( name == #X ) return X
    TRANSLATE(pointGrey_Fire_FlyMVColor);
    TRANSLATE(pointGrey_Fire_FlyMVMono);
    TRANSLATE(sony_DFW_VL500_2_30);
    TRANSLATE(apple_ISight);
    TRANSLATE(fireI_1_2);
    TRANSLATE(imagingSource_DFx_21BF04);
    TRANSLATE(pointGrey_Flea2_08S2C);
    TRANSLATE(pointGrey_Flea2_03S2M);
    TRANSLATE(pointGrey_Flea2_03S2C);
    TRANSLATE(pointGrey_Flea2G_13S2CC);
    
#undef TRANSLATE 
    else return unknownCameraType;
  }

  // }}}
#endif

  std::string DCDevice::getTypeID(const std::string &model, const std::string &vendor){
    // {{{ open

    return vendor + " -- " + model;
  }

  // }}}

  std::string DCDevice::getTypeID(const dc1394camera_t *cam){
    // {{{ open
    if(!cam) return "";
    return getTypeID(cam->model, cam->vendor);
  }

  // }}}


  /************************************* old *******************************
  DCDevice::CameraTypeID DCDevice::estimateCameraType(dc1394camera_t *cam){

    if(!cam){
      return unknownCameraType;
    }else if( is_firefly_mono(cam) ){
      return pointGrey_Fire_FlyMVMono;
    }else if( is_firefly_color(cam) ){
      return pointGrey_Fire_FlyMVColor;
    }else if( string(cam->model) == "DFW-VL500 2.30"){
      return sony_DFW_VL500_2_30;
    }else if( string(cam->vendor) == "Apple Computer, Inc."){
      return apple_ISight;
    }else if( string(cam->model) == "Fire-i 1.2"){
      return fireI_1_2;
    }else if( string(cam->model) == "DFx 21BF04"){
      return imagingSource_DFx_21BF04;
    }else if( string(cam->model) == "Flea2 FL2-08S2C" ){
      return pointGrey_Flea2_08S2C;
    }else if( string(cam->model) == "Flea2 FL2-03S2M" ){
      return pointGrey_Flea2_03S2M;
    }else if( string(cam->model) == "Flea2 FL2-03S2C" ){
      return pointGrey_Flea2_03S2C;
    }else if( string(cam->model) == "Flea2 FL2G-13S2C" ){
      return pointGrey_Flea2G_13S2CC;
    }else{
      static set<string> warned;
      if(warned.find(cam->model) == warned.end()){
        warned.insert(cam->model);
        ERROR_LOG("unknown camera model:" << cam->model);
      }
      return unknownCameraType;
    }  
  }
  bool DCDevice::needsBayerDecoding() const{
    if(isNull()) return false; 
    switch(m_eCameraTypeID){
      case pointGrey_Fire_FlyMVMono: 
      case apple_ISight:
      case sony_DFW_VL500_2_30:
      case fireI_1_2:
      case pointGrey_Flea2_08S2C:
      case pointGrey_Flea2_03S2C:
      case pointGrey_Flea2_03S2M:
      case pointGrey_Flea2G_13S2CC:
        return false;
      case pointGrey_Fire_FlyMVColor:
        return true;
      case imagingSource_DFx_21BF04:        
        if(getMode().videomode == DC1394_VIDEO_MODE_640x480_YUV422){
          return false;
        }else{
          return true;
        }
      case unknownCameraType:
      default:{
        //        ERROR_LOG("unknown camera type ID");
        //return false;
        static set<string> warned;
        if(warned.find(getUniqueStringIdentifier()) == warned.end()){
          warned.insert(getUniqueStringIdentifier());
          ERROR_LOG("unknown camera type ID:" << getUniqueStringIdentifier() << ") assuming no bayer encoding!");
        }
        return false;
      }
    }
  }
***********************************************************************/
  // **NEW-CAM**
  dc1394color_filter_t DCDevice::getBayerFilterLayout() const{
    // {{{ open

    switch(m_eBayerFilterMode){
      case BF_RGGB:
      case BF_GBRG:
      case BF_GRBG:
      case BF_BGGR: 
        return (dc1394color_filter_t)m_eBayerFilterMode;
      case BF_FROM_FEATURE:
        return (dc1394color_filter_t)1;
      case BF_FROM_MODE:
        if(getTypeID(m_poCam) == "Imaging Source -- DFx_21BF04"){
          if(getMode().videomode != DC1394_VIDEO_MODE_640x480_YUV422){
            return DC1394_COLOR_FILTER_GBRG;
          }else{
            return (dc1394color_filter_t)0;
          }
        }
      case BF_NONE: 
      default:
        return (dc1394color_filter_t)0;
    }
  }

  // }}}
  
  // **NEW-CAM**   
  void DCDevice::estimateBayerFilterMode(){
    // {{{ open
    
    if(isNull()) m_eBayerFilterMode = BF_NONE;
    std::string id = getTypeID(m_poCam);
    if(id== "Point Grey Research -- Firefly MV FFMV-03MTC"){
      m_eBayerFilterMode = BF_GBRG;
    }else if("Imaging Source -- DFx_21BF04"){
      m_eBayerFilterMode = BF_FROM_MODE;
    }else{
      // this is a list of builtin cameras that dont't have
      // a bayer filter 
      static std::set<std::string> builtin;
      if(!builtin.size()){
        builtin.insert("SONY -- DFW-VL500 2.30");
        builtin.insert("Point Grey Research -- Firefly MV FFMV-03MTM");
        builtin.insert("Apple Computer, Inc. -- iSight");
        builtin.insert("Fire-i 1.2-Vendor -- Fire-i 1.2");
        builtin.insert("Point Grey Research -- Flea2 FL2-08S2C");
        builtin.insert("Point Grey Research -- Flea2 FL2-03S2M");
        builtin.insert("Point Grey Research -- Flea2 FL2-03S2C");
        builtin.insert("Point Grey Research -- Flea2 FL2G-13S2C");
      }
      if(std::find(builtin.begin(),builtin.end(),id) != builtin.end()){
        m_eBayerFilterMode = BF_NONE;
      }else{
        m_eBayerFilterMode = BF_FROM_FEATURE;
      }
    }
  }

  // }}}


  void DCDevice::dc1394_reset_bus(bool verbose){
    // {{{ open

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

  // }}}
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
    // {{{ open

    if(isNull()) return -1;
    return m_poCam->guid;
  }

  // }}}
  icl32s DCDevice::getUnit() const{
    // {{{ open

    if(isNull()) return -1;
    return m_poCam->unit;
  }

  // }}}
  icl32s DCDevice::getUnitSpecID() const{
    // {{{ open

    if(isNull()) return -1;
    return m_poCam->unit_spec_ID;
  }

  // }}}
  std::string DCDevice::getUniqueStringIdentifier() const{
    // {{{ open

    if(isNull()) return "null";
    union GUID{
      uint64_t t64;
      uint32_t t32[2];
    } guid;
    guid.t64 = getGUID();

    return getModelID()+"-"+str(guid.t32[0])+"."+str(guid.t32[1]);
    
  }

  // }}}
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
