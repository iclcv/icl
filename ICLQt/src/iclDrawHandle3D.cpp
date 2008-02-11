#include "iclDrawHandle3D.h"
#include <iclDrawWidget3D.h>

namespace icl{
  void DrawHandle3D::setImage(const ImgBase *image){
    (**this)->setImage(image);
  }
  void DrawHandle3D::update(){
    (**this)->updateFromOtherThread();
  }
  
}
