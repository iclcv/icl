#include "iclDCGrabber.h"
#include "iclDCGrabberThread.h"
#include <iclSignalHandler.h>
#include <iclIO.h>
#include <dc1394/iso.h>

namespace icl{
  using namespace std;
  using namespace icl::dc;
  
  DCGrabberImpl::DCGrabberImpl(const DCDevice &dev, int isoMBits, bool suppressDoubledImages):
    // {{{ open

    m_oDev(dev),m_oDeviceFeatures(dev),m_poGT(0),m_poImage(0),
    m_poImageTmp(0), m_bSuppressDoubledImages(suppressDoubledImages)
  {
    
    dc::install_signal_handler();

    m_oOptions.bayermethod = DC1394_BAYER_METHOD_BILINEAR;
    
    m_oOptions.framerate = (dc1394framerate_t)-1; // use default
    
    m_oOptions.videomode = (dc1394video_mode_t)-1; // use default
    
    m_oOptions.enable_image_labeling = false;
    
    m_oOptions.isoMBits = isoMBits;
    
  }

  // }}}
  
  const ImgBase *DCGrabberImpl::grab (ImgBase **ppoDst){
    // {{{ open

    if(ppoDst){
      ERROR_LOG("Giving DCGrabber a destination image != NULL is currently not supported");
      return 0;
    }

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
  
  DCGrabberImpl::~DCGrabberImpl(){
    // {{{ open
    if(m_poGT){
      m_poGT->stop();
      ICL_DELETE(m_poGT);
    }
    ICL_DELETE(m_poImage);
    ICL_DELETE(m_poImageTmp);
    release_dc_cam(m_oDev.getCam());
  }

  // }}}

  
  std::vector<DCDevice> DCGrabberImpl::getDeviceList(bool resetBusFirst){
    // {{{ open
    if(resetBusFirst){
      DCGrabberImpl::dc1394_reset_bus(false);
    }
    std::vector<DCDevice> v;
    
    dc1394_t *context = get_static_context();
    dc1394camera_list_t *list = 0;
    dc1394error_t err = dc1394_camera_enumerate(context,&list);
    if(err != DC1394_SUCCESS){ 
      if(list){
        dc1394_camera_free_list(list);
      }
      ERROR_LOG("Unable to create device list: returning empty list!");
      return v;
    }
    if(!list){
      ERROR_LOG("no dc device found!");
      return v;
    }

    for(uint32_t i=0;i<list->num;++i){
      v.push_back(DCDevice(dc1394_camera_new_unit(context,list->ids[i].guid,list->ids[i].unit)));

      //std::cout << "trying to release all former iso data flow for camera " << v.back().getCam() << std::endl;
      //dc1394_iso_release_all(v.back().getCam());
      
      if(!i){
        // This is very hard so when an icl application is started, 
        // it will reset the bus first ??
        //dc1394_reset_bus(v.back().getCam());
      }
                   
    }
    
    if(list){
      dc1394_camera_free_list(list);
    }    
    return v;
  }

  // }}}
  
  void DCGrabberImpl::restartGrabberThread(){
    if(m_poGT){
      m_poGT->stop();
      //      m_poGT->waitFor();
      delete m_poGT;
    }
    m_poGT = new DCGrabberThread(m_oDev.getCam(),&m_oOptions, m_bSuppressDoubledImages);
    m_poGT->start();
    usleep(10*1000);
  }

  void DCGrabberImpl::setProperty(const std::string &property, const std::string &value){
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
    }else if(m_oDeviceFeatures.supportsProperty(property)){
      m_oDeviceFeatures.setProperty(property,value);
    }else{
      ERROR_LOG("unsupported property " << property);
    }
  }
  std::vector<std::string> DCGrabberImpl::getPropertyList(){
    std::vector<std::string> v;
    if(m_oDev.isNull()) return v;
    
    if(m_oDev.needsBayerDecoding()){
      v.push_back("bayer-quality");
    }
    v.push_back("format");
    v.push_back("size");
    v.push_back("enable-image-labeling");
    
    std::vector<std::string> v3 = m_oDeviceFeatures.getPropertyList();
    std::copy(v3.begin(),v3.end(),back_inserter(v));
    
    return v;
  }

  std::string DCGrabberImpl::getType(const std::string &name){
    if((m_oDev.needsBayerDecoding() && name == "bayer-quality") || 
       name == "format" || name == "size" || name == "enable-image-labeling"){
      return "menu";
    }else if(m_oDeviceFeatures.supportsProperty(name)){
      return m_oDeviceFeatures.getType(name);
    }
    return "";// range command undefined
  }
 
  std::string DCGrabberImpl::getInfo(const std::string &name){
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
    }else if(m_oDeviceFeatures.supportsProperty(name)){
      return m_oDeviceFeatures.getInfo(name);
    }
    return "";
  }
  std::string DCGrabberImpl::getValue(const std::string &name){
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
    }else if(m_oDeviceFeatures.supportsProperty(name)){
      return m_oDeviceFeatures.getValue(name);
    }
    return "";
  }

  
  
}
  

