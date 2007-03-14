#include <iclUnicapGrabber.h>
#include <iclQuick.h>


int main(){
  UnicapGrabber g;
  Img8u *image= g.grab()->convert<icl8u>();
  
  ImgQ a = cvt(*image);
  label(a,"grabbed image");
  show(a);
  
}
