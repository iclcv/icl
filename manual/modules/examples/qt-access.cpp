#include <ICLQt/Common.h>
#include <ICLFilter/UnaryCompareOp.h>

GUI gui;                 
GenericGrabber grabber;  
UnaryCompareOp cmp(">"); 

void init(){
  gui << Image().handle("image")
      << Slider(0,255,127).handle("thresh").label("threshold").maxSize(100,2)
      << Show();
  grabber.init(pa("-i"));
}

void run(){
  // extract current slider value
  cmp.setValue(gui["thresh"]);
  
  // set displayed image
  gui["image"] = cmp.apply(grabber.grab());
}
int main(int n, char **args){
  return ICLApp(n,args,"-input|-i(2)",init,run).exec();
}
