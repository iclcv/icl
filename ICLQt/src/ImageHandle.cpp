#include <ICLQt/ImageHandle.h>
#include <ICLQt/Widget.h>

namespace icl{
  void ImageHandle::setImage(const ImgBase *image){
    (**this)->setImage(image);
  }
  void ImageHandle::update(){
    (**this)->updateFromOtherThread();
  }

  void ImageHandle::registerCallback(GUI::CallbackPtr cb, const std::string &events){
    (**this)->registerCallback(cb,events);
  }
  
  void ImageHandle::removeCallbacks(){
    (**this)->removeCallbacks();
  }

  
}
