#include "iclDCDevice.h"
#include <iclStrTok.h>
#include <iclMacros.h>
#include <iclDC.h>

using std::vector;
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

  std::string DCDevice::getVendorID() const{
    if(isNull()) return "null";  
    return m_poCam->vendor;
  }
  
  std::string DCDevice::getModelID() const{
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
  void DCDevice::show(const std::string &title) const{
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
    }else{
      ERROR_LOG("unsupported camera!");
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
    }else{
      ERROR_LOG("unsupported camera!");
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



  DCDevice::Mode::Mode(const std::string &stringRepr){
    // {{{ open

    StrTok s(stringRepr,"@");
    const std::vector<std::string> &toks = s.allTokens();
    ICLASSERT(toks.size() == 2);
    videomode = dc::videomode_from_string(toks[0]);
    framerate = dc::framerate_from_string(toks[1]);
    
  }

  // }}}
  std::string DCDevice::Mode::toString() const{
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
