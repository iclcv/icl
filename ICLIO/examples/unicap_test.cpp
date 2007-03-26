#include <iclUnicapGrabber.h>
#include <iclQuick.h>
#include <iclStackTimer.h>

void grab_single_frame(UnicapGrabber &g){
  BENCHMARK_THIS_FUNCTION;
  static ImgBase *image = new Img8u(Size(640,480),formatRGB);
  static const Img8u *image2 = 0;
  image2 = g.grab(&image)->asImg<icl8u>();
}

void grab_100_frames(UnicapGrabber &g){
  BENCHMARK_THIS_FUNCTION;
  for(int i=0;i<100;i++){
    grab_single_frame(g);
  }
}

int main(){
  const std::vector<UnicapDevice> l = UnicapGrabber::getDeviceList();
  if(!l.size()) {
    ERROR_LOG("alles ist woanders !");
     exit(-1001);
  }
  UnicapGrabber g(l[0]);
  //  l[0].listFormats();
  //l[0].listProperties();
  g.setParam("frame rate","30");
  
  //UnicapGrabber g("device=/dev/video1394-0");
  g.setDesiredParams(ImgParams(Size(640,480),formatRGB));
  g.setDesiredDepth(depth8u);

  grab_100_frames(g);
  
  //  ImgQ a = cvt(*image);
  //label(a,"grabbed image");
  //show(a);
  
}
