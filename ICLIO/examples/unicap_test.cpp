#include <iclUnicapGrabber.h>
#include <iclQuick.h>


int main(){
  const std::vector<UnicapDevice> l = UnicapGrabber::getDeviceList();
  if(!l.size()) {
    ERROR_LOG("alles ist woanders !");
     exit(-1001);
  }
  UnicapGrabber g(l[0]);
  l[0].listFormats();
  l[0].listProperties();
  
  //UnicapGrabber g("device=/dev/video1394-0");
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
