#include <ICLQuick/Common.h>

GUI gui;

void init(){
  gui << "image[@handle=image@minsize=16x12]"
      << "combo(Gray,RGB,HLS,YUV,LAB,Chroma,Matrix)"
         "[@out=cs@label=color space@maxsize=100x3]";
  
  gui.show();
}

void run(){
  gui_string(cs);
  static GenericGrabber g(FROM_PROGARG("-input"));
  g.setIgnoreDesiredParams(true);
  g.setDesiredSize(g.grab()->getSize());
  g.setDesiredFormat(parse<format>(cs));
  g.setIgnoreDesiredParams(false);
  
  gui["image"] = g.grab();
  gui["image"].update();
}

int main(int n, char **args){
  return ICLApplication(n,args,"-input(2)",init,run).exec();
}
