#include <ICLQt/Common.h>
#include <ICLGeom/Plot3D.h>


GUI gui;
Scene scene;

void init(){
  TODO_LOG("move the Plot3D GUIComponent explicitly to the Geom package. Now it "
           "cannot be included by Qt/Common, but by the Geom-Header. The the linkage"
           "show always be correct");
  
  gui << Plot3D().handle("plot") << Show();
}

void run(){
  PlotHandle3D plot = gui["plot"];
  Thread::msleep(1000);
}

int main(int n, char **a){
  return ICLApp(n,a,"",init,run).exec();
}
