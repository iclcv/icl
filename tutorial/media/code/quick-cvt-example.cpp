#include <iclQuick.h>

int main(){
  /// create original image
  ImgQ x = create("parrot");

  /// convert into special types
  Img8u image8u = cvt8u(x);
  Img16s image16s = cvt16s(x);
  Img32s image32s = cvt32s(x);
  Img32f image32f = cvt32f(x);
  Img64f image64f = cvt64f(x);
  
  /// re-convert into ImgQ
  ImgQ qs[5] = {
    cvt(image8u),
    cvt(image16s),
    cvt(image32s),
    cvt(image32f),
    cvt(image64f)
  };
  
  /// or for pointers:
  Img8u *p8u = new Img8u(Size::VGA,1);
  ImgBase *pb = new Img64f(Size::QVGA,formatRGB);
  
  /// and finally reconvert to ImgQ
  ImgQ i8u = cvt(p8u);
  ImgQ ib = cvt(pb);
}
