#include <iclUnicapGrabber.h>
#include <iclQuick.h>


int main(){
  UnicapGrabber g("/dev/video0");
  g.setDesiredParams(ImgParams(Size(640,480),formatRGB));
  g.setDesiredDepth(depth8u);

  const Img8u *image;
  for(int i=0;i<50;i++){
    image = g.grab((ImgBase**)0)->asImg<icl8u>();
  }
  
  ImgQ a = cvt(*image);
  label(a,"grabbed image");
  show(a);
  
}
