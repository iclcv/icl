#include "iclDrawHandle.h"
#include <iclDrawWidget.h>

namespace icl{
  void DrawHandle::setImage(const ImgBase *image){
    (**this)->setImage(image);
  }
  void DrawHandle::update(){
    (**this)->updateFromOtherThread();
  }
  void DrawHandle::registerCallback(GUI::CallbackPtr cb, const std::string &events){
    (**this)->registerCallback(cb,events);
  }
  void DrawHandle::removeCallbacks(){
    (**this)->removeCallbacks();
  }
  
}
