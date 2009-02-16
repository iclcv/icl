#include <iclCommon.h>

GUI gui;
void run(){
  static GenericGrabber g(FROM_PROGARG("-input"));
  g.setDesiredSize(translateSize(pa_subarg<std::string>("-size",0,"VGA")));
  if(pa_defined("-dist")){
    g.enableDistortion(DIST_FROM_PROGARG("-dist"),g.getDesiredSize());
  }
  while(true){
    gui.getValue<ImageHandle>("image") = g.grab();
    gui.getValue<ImageHandle>("image").update();
    Thread::msleep(10);
  }
}

int main(int n, char**ppc){
  ExecThread x(run);
  pa_explain("-input","define input grabber parameters\ne.g. -dc 0 or -file *.ppm");
  pa_explain("-dist","define for parameters for radial distortion.\n"
             "parameters can be obained running 'icl-calib-radial-distortion'");
  pa_explain("-size","desired image size of grabber");

  pa_init(n,ppc,"-input(2) -dist(4) -size(1)");
  if(!pa_defined("-input")){
    pa_usage("please define argument \"-input\"");
    exit(-1);
  }
  QApplication app(n,ppc);
  gui << "image()[@handle=image@minsize=16x12]";
  gui.show();

  x.run();
  
  return app.exec();
}
