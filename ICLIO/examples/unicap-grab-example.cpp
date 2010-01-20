#include <ICLIO/UnicapGrabber.h>
#include <ICLQuick/Quick.h>
#include <ICLUtils/StackTimer.h>

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
  std::vector<UnicapDevice> l = UnicapGrabber::getDeviceList();
  printf ("devices: %d\n", l.size());
  l = UnicapGrabber::getDeviceList();
  printf ("devices: %d\n", l.size());
  l = UnicapGrabber::getDeviceList();
  printf ("devices: %d\n", l.size());
  if(!l.size()) {
     ERROR_LOG("no devices were found !");
     exit(-1001);
  }
  UnicapGrabber g(l[0]);
  //  l[0].listFormats();
  l[0].listProperties();
  //  g.setProperty("frame rate","30");
  
  //UnicapGrabber g("device=/dev/video1394-0");
  g.setDesiredParams(ImgParams(Size(640,480),formatRGB));
  g.setDesiredDepth(depth8u);

  grab_100_frames(g);
  
  //  ImgQ a = cvt(*image);
  //label(a,"grabbed image");
  //show(a);
  
}
