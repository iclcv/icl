#include <ICLFilter/WienerOp.h>
#include <ICLQuick/Quick.h>

int main(){
  ImgQ src = create("parrot");

  ImgBase *dst = 0;
  src.setROI(Rect(20,20,600,800));
  
  WienerOp wo(Size(5,5),0.2);
  wo.setClipToROI(true);
  wo.apply (&src,&dst);
  show(cvt(dst));
}
