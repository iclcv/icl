#include "iclDrawHandle.h"
#include <iclDrawWidget.h>

namespace icl{
  void DrawHandle::setImage(const ImgBase *image){
    (**this)->setImage(image);
  }
  void DrawHandle::update(){
    (**this)->updateFromOtherThread();
  }
  
}
