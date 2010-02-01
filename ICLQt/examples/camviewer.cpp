#include <ICLQuick/Common.h>
#include <ICLUtils/FPSLimiter.h>

GUI gui;
FPSLimiter *fpsLimiter = 0;
void run(){
  static GenericGrabber g(FROM_PROGARG("-input"));
  g.setDesiredSize(pa("-size"));
  if(pa("-dist")){
    g.enableDistortion(DIST_FROM_PROGARG("-dist"),g.getDesiredSize());
  }
  while(true){
    gui["image"] = g.grab();
    gui["image"].update();
    if(pa("-showfps")){
      gui["fps"].update();
    }
    fpsLimiter->wait();
  }
}

void init(){
  gui << "image()[@handle=image@minsize=16x12]";
  if(pa("-showfps")){
    gui << "fps(10)[@handle=fps@maxsize=100x2@minsize=5x2]";
  }
  gui.show();
  
  fpsLimiter = new FPSLimiter(pa("-maxfps"),10);

  if(pa("-bci-auto")){
    (*gui.getValue<ImageHandle>("image"))->setRangeMode(ICLWidget::rmAuto);
  }
}

int main(int n, char**ppc){
  paex
  ("-input","define input grabber parameters\ne.g. -dc 0 or -file *.ppm")
  ("-dist","define for parameters for radial distortion.\n"
   "parameters can be obained running 'icl-calib-radial-distortion'")
  ("-size","desired image size of grabber")
  ("-bci-auto","set visualization window to auto bci-mode (brightness-contrast-adaption)");
  return ICLApp(n,ppc,"[m]-input|-i(device,device-params) "
                "-dist|-d(float=0,float=0,float=0,float=0) "
                "-size|-s(Size?VGA) -showfps -maxfps(float=30) "
                "-bci-auto",init,run).exec();
}
