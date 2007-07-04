#include "iclDCDevice.h"
#include <iclStrTok.h>
#include <iclMacros.h>

using std::vector;

namespace icl{

  const DCDevice DCDevice::null = DCDevice(0);
  
  vector<DCDevice::Mode> DCDevice::getModes() const{
    // {{{ open

    vector<DCDevice::Mode> v;
    ICLASSERT_RETURN_VAL( !isNull(), v);
                 
    dc1394video_modes_t modeList;
    dc1394_video_get_supported_modes(m_poCam,&modeList);
    for(unsigned int i=0;i<modeList.num;i++){
      dc1394framerates_t framerateList;
      dc1394_video_get_supported_framerates(m_poCam,modeList.modes[i],&framerateList);
      for(unsigned int j=0;j<framerateList.num;j++){
        v.push_back(DCDevice::Mode(modeList.modes[i],framerateList.framerates[j]));
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
