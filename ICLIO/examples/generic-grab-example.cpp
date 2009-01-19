#include <iclGenericGrabber.h>
#include <iclQt.h>
#include <iclGUI.h>
#include <iclQuick.h>
#include <iclIO.h>

GUI gui("vbox");

void init(){
  gui << "image[@minsize=32x24@handle=image]" 
      << "combo(pwc,dc,unicap,file,demo)[@handle=combo@out=xxx]";
  
  gui.show();
}

void loop(){
  static ComboHandle &combo = gui.getValue<ComboHandle>("combo");
  static ImageHandle &image = gui.getValue<ImageHandle>("image");
  static GenericGrabber *grabber = 0;
  static std::map<string,string> params;
  params["pwc"] = "pwc=0";
  params["dc"] = "dc=0";
  params["unicap"] = "unicap=";
  params["file"] = "file=images/*.ppm";
  params["demo"] = "";
  while(1){
    static std::string lastComboItem = "";
    std::string type = combo.getSelectedItem();

    if(lastComboItem != type && (!grabber || grabber->getType() != type)){
      ICL_DELETE( grabber );
      grabber = new GenericGrabber(type,params[type],false);
      if(grabber->isNull()){
        ICL_DELETE( grabber );
      }
    }
    
    if(grabber){
      image = grabber->grab();
    }else{
      ImgQ buf = zeros(640,480,3);
      labelImage(&buf,string("no ")+type+"-grabber available");
      image = &buf;
    }

    image.update();
    Thread::msleep(20);  
    lastComboItem = type;  
  }
}



int main(int n, char **ppc){
  ExecThread x(loop);
  
  QApplication app(n,ppc);
  
  init();

  x.run();

  return app.exec();
}
