#include "iclDCGrabber.h"
#include "iclDCGrabberThread.h"


namespace icl{
  using namespace icl::dc;
  
  DCGrabber::DCGrabber():m_poGT(0),m_poImage(0){}
  
  void DCGrabber::initialize(){
    if(!m_poGT){
      dc1394camera_t **ppoCams;
      uint32_t numCams=0;
      dc1394_find_cameras(&ppoCams,&numCams);
      ICLASSERT_THROW(numCams, ICLException("no device found!") );
      dc1394_print_camera_info(ppoCams[0]);
      
      m_poCam = ppoCams[0];
      
      m_poGT = new DCGrabberThread(ppoCams[0]);
      m_poGT->start();
    }  
  }
  
  const ImgBase *DCGrabber::grab (ImgBase **ppoDst){
    initialize();
    if(ppoDst){
      m_poGT->getCurrentImage(ppoDst);    
      return *ppoDst;
    }else{
      ppoDst = &m_poImage;
      m_poGT->getCurrentImage(ppoDst);    
      return m_poImage;
    }
    
  }

  DCGrabber::~DCGrabber(){
    m_poGT->stop();   
    m_poGT->waitFor();
    ICL_DELETE(m_poGT);
  }
  
  
}
  

