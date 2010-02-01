#include <ICLQuick/Common.h>
#include <ICLUtils/ConfigFile.h>
#include <ICLQt/ConfigEntry.h>

GUI gui("hbox");
GenericGrabber *grabber = 0;

void init(){
  ConfigFile cfg;
  cfg.set("config.threshold",int(3));
  cfg.setRestriction("config.threshold",ConfigFile::KeyRestriction(0,255));
  ConfigFile::loadConfig(cfg);


  gui << "image[@handle=image@minsize=32x24@label=image]";
  
  GUI con("vbox");
  con << "config(embedded)[@label=configuration@minsize=15x15]";
  con << "fps(50)[@handle=fps]";

  gui << con;
  gui.show();
  
  

  grabber = new GenericGrabber(FROM_PROGARG("-input"));
  grabber->setDesiredSize(Size::VGA);
}



void run(){
  ImgQ image = cvt(grabber->grab());

  ImgQ a = gray(image);
  
  static ConfigEntry<int> t("config.threshold");

  ImgQ b = thresh(a,t);

  ImgQ c = thresh(a,2*t);
  
  ImgQ d = binXOR<icl8u>(b,c);
  
  ImgQ e = (b,c,d);
  
  ImgQ f = thresh(b,128);
  
  ImgQ g = (255.0/c+d-b+4*0.3);
  
  ImgQ h = (e%g);

  h.setROI(Rect(300,300,332,221));
  
  ImgQ i = copyroi(h);
  
  static ImageHandle han = gui.getValue<ImageHandle>("image");
  han = i;
  han.update();

  static FPSHandle fps = gui.getValue<FPSHandle>("fps");
  fps.update();
  
  
}


int main(int n,char **ppc){
  return ICLApp(n,ppc,"-input|-i(device,device-params)",init,run).exec();
}
