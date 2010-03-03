#include <ICLIO/CreateGrabber.h>
#include <ICLIO/TestImages.h>

namespace icl{

  const ImgBase* CreateGrabberImpl::grabUD(ImgBase **ppoDst){
    if(getIgnoreDesiredParams()){
      if(!ppoDst) return m_image;
      if(!*ppoDst) *ppoDst = m_image->deepCopy();
      else m_image->deepCopy(ppoDst);
    }else{
      ImgBase *image = prepareOutput(ppoDst);
      m_oConverter.apply(m_image,image);
      return image;
    }
    return 0;
  }

  CreateGrabberImpl::CreateGrabberImpl(const std::string &what){
    m_image = TestImages::create(what);
    if(!m_image) throw ICLException("unable to create a 'CreateGrabber' from given description '"+what+"'");
  }
  CreateGrabberImpl::~CreateGrabberImpl(){
    ICL_DELETE(m_image);
  }

}
    
