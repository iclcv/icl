#include <ICLQt/Common.h>
#include <ICLGeom/Plot3D.h>


GUI gui;
Scene scene;

void init(){
  gui << Plot3D().handle("plot").minSize(32,24) << Show();
}

void run(){
  PlotHandle3D plot = gui["plot"];
  Thread::msleep(1000);
}

int main(int n, char **a){
  
  
  
  return ICLApp(n,a,"",init,run).exec();
}
