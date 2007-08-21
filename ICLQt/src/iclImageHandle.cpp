#include "iclImageHandle.h"
#include <iclWidget.h>

namespace icl{
  void ImageHandle::setImage(const ImgBase *image){
    (**this)->setImage(image);
  }
  void ImageHandle::update(){
    (**this)->update();
  }
  
}
