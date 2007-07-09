#include "iclDCGrabber.h"
#include "iclDCGrabberThread.h"
#include <iclSignalHandler.h>
#include <iclIO.h>

namespace icl{
  using namespace std;
  using namespace icl::dc;
  
  DCGrabber::DCGrabber(const DCDevice &dev):
    // {{{ open

    m_oDev(dev),m_poGT(0),m_poImage(0), m_poImageTmp(0)
  {
    dc::install_signal_handler();

    m_eBayerMethod = DC1394_BAYER_METHOD_BILINEAR;
  }

  // }}}
  
  const ImgBase *DCGrabber::grab (ImgBase **ppoDst){
    // {{{ open

    ICLASSERT_RETURN_VAL( !m_oDev.isNull(), 0);
    if(!m_poGT){
      m_oDev.reset();
      m_poGT = new DCGrabberThread(m_oDev.getCam());
      m_poGT->start();
      usleep(10*1000);
    }
    
    ppoDst = ppoDst ? ppoDst : &m_poImage;
    ImgBase **ppoDstTmp = &m_poImageTmp;
    bool desiredParamsFullfilled = false;
    
    m_poGT->getCurrentImage(ppoDst,ppoDstTmp,desiredParamsFullfilled,
                            getDesiredSize(),getDesiredFormat(), getDesiredDepth(),
                            bayermethod_from_string(getValue("bayer-quality")) );
    
    if(!desiredParamsFullfilled){
      ensureCompatible(ppoDst,getDesiredDepth(),getDesiredParams());
      m_oConverter.apply(*ppoDstTmp,*ppoDst);
    }
    
    labelImage(*ppoDst,m_oDev.getModelID());
    
    return *ppoDst;
  }

  // }}}
  
  DCGrabber::~DCGrabber(){
    // {{{ open

    m_poGT->stop();
    ICL_DELETE(m_poGT);
  }

  // }}}

  std::vector<DCDevice> DCGrabber::getDeviceList(){
    // {{{ open

    std::vector<DCDevice> v;

    dc1394camera_t **ppoCams;
    uint32_t numCams=0;
    dc1394_find_cameras(&ppoCams,&numCams);
    
    for(uint32_t i=0;i<numCams;i++){
      v.push_back(DCDevice(ppoCams[i]));
    }
    return v;
  }

  // }}}


  void DCGrabber::setProperty(const std::string &property, const std::string &value){
    if(property == "bayer-quality"){
      m_eBayerMethod = bayermethod_from_string(value);
    }
  }
  std::vector<std::string> DCGrabber::getPropertyList(){
    std::vector<std::string> v;
    v.push_back("bayer-quality");
    return v;
  }
  bool DCGrabber::supportsProperty(const std::string &property){
    if(property == "bayer-quality") return true;
    return false;
  }
  std::string DCGrabber::getType(const std::string &name){
    if(name == "bayer-quality") return "menu";
    return "";// range command undefined
  }
  std::string DCGrabber::getInfo(const std::string &name){
    if(name == "bayer-quality"){
      return "{"
      "\"DC1394_BAYER_METHOD_NEAREST\","
      "\"DC1394_BAYER_METHOD_BILINEAR\","
      "\"DC1394_BAYER_METHOD_HQLINEAR\","
      "\"DC1394_BAYER_METHOD_DOWNSAMPLE\","
      "\"DC1394_BAYER_METHOD_EDGESENSE\","
      "\"DC1394_BAYER_METHOD_VNG\","
      "\"DC1394_BAYER_METHOD_AHD\"}";
    }
    return "";
  }
  std::string DCGrabber::getValue(const std::string &name){
    if(name == "bayer-quality") return to_string(m_eBayerMethod);
    return "";
  }

  
  
}
  

