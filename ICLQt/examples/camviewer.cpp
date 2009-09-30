#include <iclCommon.h>
#include <iclFPSLimiter.h>

GUI gui;
FPSLimiter *fpsLimiter = 0;
void run(){
  static GenericGrabber g(FROM_PROGARG("-input"));
  g.setDesiredSize(parse<Size>(pa_subarg<std::string>("-size",0,"VGA")));
  if(pa_defined("-dist")){
    g.enableDistortion(DIST_FROM_PROGARG("-dist"),g.getDesiredSize());
  }
  while(true){
    gui["image"] = g.grab();
    gui["image"].update();
    if(pa_defined("-showfps")){
      gui["fps"].update();
    }
    if(fpsLimiter){
      fpsLimiter->wait();
    }else{
      Thread::msleep(5);
    }
  }
}

void init(){
  if(!pa_defined("-input")){
    pa_usage("please define argument \"-input\"");
    exit(-1);
  }
  gui << "image()[@handle=image@minsize=16x12]";
  if(pa_defined("-showfps")){
    gui << "fps(10)[@handle=fps@maxsize=100x2@minsize=5x2]";
  }
  gui.show();
  
  if(pa_defined("-maxfps")){
    fpsLimiter = new FPSLimiter(pa_subarg<float>("-maxfps",0,30));
  }
  if(pa_defined("-bci-auto")){
    (*gui.getValue<ImageHandle>("image"))->setRangeMode(ICLWidget::rmAuto);
  }
}

int main(int n, char**ppc){
  pa_explain("-input","define input grabber parameters\ne.g. -dc 0 or -file *.ppm");
  pa_explain("-dist","define for parameters for radial distortion.\n"
             "parameters can be obained running 'icl-calib-radial-distortion'");
  pa_explain("-size","desired image size of grabber");
  pa_explain("-bci-auto","set visualization window to auto bci-mode (brightness-contrast-adaption)");
  return ICLApplication(n,ppc,"-input(2) -dist(4) -size(1) -showfps -maxfps(1) -bci-auto",init,run).exec();
}
