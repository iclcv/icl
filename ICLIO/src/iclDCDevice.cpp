#include "iclDCDevice.h"
#include <iclStrTok.h>
#include <iclMacros.h>
#include <iclDC.h>
#include <stdio.h>
#include <iclThread.h>

#include <algorithm>
#include <iclStringUtils.h>

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
      TRANSLATE(pointGrey_Flea2_FL2_08S2C);
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
    TRANSLATE(pointGreyFire_FlyMVMono);
    TRANSLATE(sony_DFW_VL500_2_30);
    TRANSLATE(apple_ISight);
    TRANSLATE(fireI_1_2);
    TRANSLATE(imagingSource_DFx_21BF04);
    TRANSLATE(pointGrey_Flea2_FL2_08S2C);
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
    }else if( string(cam->model) == "Flea2 FL2-08S2C" ){
      return pointGrey_Flea2_FL2_08S2C;
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

  std::string DCDevice::getUniqueStringIdentifier() const{
    if(isNull()) return "null";
    union GUID{
      uint64_t t64;
      uint32_t t32[2];
    } guid;
    guid.t64 = getGUID();

    return getModelID()+"-"+str(guid.t32[0])+"."+str(guid.t32[1]);
    
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
        return fmt == formatRGB || fmt == formatMatrix;
      case pointGrey_Flea2_FL2_08S2C:
        return fmt == formatRGB || fmt == formatMatrix; // maybe also yuv ??
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
    static const Size s10(1024,768);
    static const Size s80(800,600);
    static const Size s64(640,480);
    static const Size s32(320,240);
    static const Size s16(160,120);
    
    
    
    switch(m_eCameraTypeID){
      case pointGreyFire_FlyMVMono: 
      case imagingSource_DFx_21BF04:
        return size == s64;
      case pointGreyFire_FlyMVColor:
      case sony_DFW_VL500_2_30:
      case apple_ISight:
      case fireI_1_2:
        return size == s64 || size == s32;
      case pointGrey_Flea2_FL2_08S2C:
        return size == s10 || size == s80 || size == s64 || size == s32 || size == s16;
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
      case pointGrey_Flea2_FL2_08S2C:
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
