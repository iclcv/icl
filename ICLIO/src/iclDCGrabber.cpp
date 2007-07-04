#include "iclDCGrabber.h"
#include "iclDCGrabberThread.h"


namespace icl{
  using namespace std;
  using namespace icl::dc;
  
  DCGrabber::DCGrabber(const DCDevice &dev):
    m_oDev(dev),m_poGT(0),m_poImage(0)
  {}
  
  const ImgBase *DCGrabber::grab (ImgBase **ppoDst){
    ICLASSERT_RETURN_VAL( !m_oDev.isNull(), 0);
    if(!m_poGT){
      
      printf("using this camera: \n");
      dc1394_print_camera_info(m_oDev.getCam());
    

      m_poGT = new DCGrabberThread(m_oDev.getCam());
      m_poGT->start();
      usleep(10*1000);
    }
    
    ppoDst = ppoDst ? ppoDst : &m_poImage;
    
    m_poGT->getCurrentImage(ppoDst);    
    return *ppoDst;
  }

  DCGrabber::~DCGrabber(){
    m_poGT->stop();   
    m_poGT->waitFor();
    ICL_DELETE(m_poGT);
  }


  std::vector<DCDevice> DCGrabber::getDeviceList(){
    std::vector<DCDevice> v;

    dc1394camera_t **ppoCams;
    uint32_t numCams=0;
    dc1394_find_cameras(&ppoCams,&numCams);
    
    for(uint32_t i=0;i<numCams;i++){
      v.push_back(DCDevice(ppoCams[i]));
    }
    return v;
  }



  void DCGrabber::setProperty(const std::string &property, const std::string &value){
    (void)property; (void)value;
  }
  std::vector<std::string> DCGrabber::getPropertyList(){
    return std::vector<std::string>();
  }
  bool DCGrabber::supportsProperty(const std::string &property){
    return false;
  }
  std::string DCGrabber::getType(const std::string &name){
    (void)name;
    return "";
  }
  std::string DCGrabber::getInfo(const std::string &name){
    (void)name;
    return "";
  }
  std::string DCGrabber::getValue(const std::string &name){
    (void)name;
    return "";
  }

  
  
}
  

