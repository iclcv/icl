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

    m_oOptions.bayermethod = DC1394_BAYER_METHOD_BILINEAR;
    
    m_oOptions.framerate = (dc1394framerate_t)-1; // use default
    
    m_oOptions.videomode = (dc1394video_mode_t)-1; // use default
    
    m_oOptions.enable_image_labeling = false;
    
  }

  // }}}
  
  const ImgBase *DCGrabber::grab (ImgBase **ppoDst){
    // {{{ open

    ICLASSERT_RETURN_VAL( !m_oDev.isNull(), 0);
    if(!m_poGT){
      restartGrabberThread();
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
    if(m_oOptions.enable_image_labeling){
      labelImage(*ppoDst,m_oDev.getModelID());
    }
    return *ppoDst;
  }

  // }}}
  
  DCGrabber::~DCGrabber(){
    // {{{ open
    if(m_poGT){
      m_poGT->stop();
      ICL_DELETE(m_poGT);
    }
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
  
  void DCGrabber::restartGrabberThread(){
    if(m_poGT){
      m_poGT->stop();
      m_poGT->waitFor();
      delete m_poGT;
    }
    m_poGT = new DCGrabberThread(m_oDev.getCam(),&m_oOptions);
    m_poGT->start();
    usleep(10*1000);
  }

  void DCGrabber::setProperty(const std::string &property, const std::string &value){
    if(m_oDev.isNull()) return;
    if(property == "bayer-quality"){
      m_oOptions.bayermethod = bayermethod_from_string(value);
    }else if(property == "format"){
      DCDevice::Mode m(value);
      if(m_oDev.getMode() != m && m_oDev.supports(m)){
        m_oOptions.framerate = m.framerate;
        m_oOptions.videomode = m.videomode;
        if(m_poGT){
          restartGrabberThread();
        }
      }
    }else if(property == "enable-image-labeling"){
      if(value == "on"){
        m_oOptions.enable_image_labeling = true;
      }else if(value == "off"){
        m_oOptions.enable_image_labeling = false;
      }else{
        ERROR_LOG("parameter image-labeling has values \"on\" and \"off\", nothing known about \""<<value<<"\"");
      }
    }else if(m_oDev.isFeatureAvailable(property)){
      m_oDev.setFeatureValue(property,value);
    }else{
      ERROR_LOG("nothing known about a property named \""<<property<<"\", value was \""<<value<<"\"");
    }
  }
  std::vector<std::string> DCGrabber::getPropertyList(){
    std::vector<std::string> v;
    if(m_oDev.isNull()) return v;
    
    if(m_oDev.needsBayerDecoding()){
      v.push_back("bayer-quality");
    }
    v.push_back("format");
    v.push_back("size");
    v.push_back("enable-image-labeling");
    
    std::vector<std::string> v2 = m_oDev.getFeatures();
    for(unsigned int i=0;i<v2.size();i++){
      v.push_back(v2[i]);
    }
    return v;
  }

  std::string DCGrabber::getType(const std::string &name){
    if((m_oDev.needsBayerDecoding() && name == "bayer-quality") || 
       name == "format" ||
       name == "size" ||
       name == "enable-image-labeling") return "menu";
    if(m_oDev.isFeatureAvailable(name)){
      return m_oDev.getFeatureType(name);
    }
    return "";// range command undefined
  }
 
  std::string DCGrabber::getInfo(const std::string &name){
    if(m_oDev.isNull()) return "";
    if(m_oDev.needsBayerDecoding() && name == "bayer-quality"){
      return "{"
      "\"DC1394_BAYER_METHOD_NEAREST\","
      "\"DC1394_BAYER_METHOD_BILINEAR\","
      "\"DC1394_BAYER_METHOD_HQLINEAR\","
      "\"DC1394_BAYER_METHOD_DOWNSAMPLE\","
      "\"DC1394_BAYER_METHOD_EDGESENSE\","
      "\"DC1394_BAYER_METHOD_VNG\","
      "\"DC1394_BAYER_METHOD_AHD\"}";
    }else if(name == "format"){
      std::vector<DCDevice::Mode> mv= m_oDev.getModes();
      std::vector<string> v;
      for(unsigned int i=0;i<mv.size();i++){
        v.push_back(mv[i].toString());
      }
      return Grabber::translateStringVec(v);
    }else if(name == "enable-image-labeling"){
      return "{\"on\",\"off\"}";
    }else if(name == "size"){
      return "{\"adjusted by format\"}";
    }else if(m_oDev.isFeatureAvailable(name)){
      return m_oDev.getFeatureInfo(name);
    }
    return "";
  }
  std::string DCGrabber::getValue(const std::string &name){
    if(m_oDev.isNull()) return "";  

    if(m_oDev.needsBayerDecoding() && name == "bayer-quality") return to_string(m_oOptions.bayermethod);
    else if(name == "format"){
      if(m_oDev.isNull()) return "";
      return m_oDev.getMode().toString();
    }else if(name == "enable-image-labeling"){
      if(m_oOptions.enable_image_labeling){
        return "on";
      }else{
        return "off";
      }
    }else if(name == "size"){
      return "adjusted by format";
    }else if(m_oDev.isFeatureAvailable(name)){
      return m_oDev.getFeatureValue(name);
    }
    return "";
  }

  
  
}
  

