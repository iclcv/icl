#include <ICLQt/Common.h>
#include <ICLFilter/UnaryCompareOp.h>
#include <ICLUtils/FPSLimiter.h>

GUI gui;
GenericGrabber grabber;
UnaryCompareOp cmp(">");
FPSLimiter fps(25);

void init(){
  gui << Display().handle("image")
      << Slider(0,255,127).handle("thresh")
         .label("threshold").maxSize(100,2)
      << Show();
  grabber.init(pa("-i"));
}

void run(){
  cmp.setValue(gui["thresh"]);
  gui["image"] = cmp.apply(grabber.grabImage());
  fps.wait();
}
int main(int n, char **args){
  return ICLApp(n,args,"-input|-i(2)",
                init,run).exec();
}
