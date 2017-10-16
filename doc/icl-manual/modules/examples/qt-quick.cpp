#include <ICLQt/Common.h>

GUI gui;
GenericGrabber grabber;
ImgQ last;  // last image

void init(){
  grabber.init(pa("-i"));

  gui << Image().handle("image")
      << Slider(0,255,127).out("thresh")
      << Show();
}

void run(){
  // use cvt to create an Img32 (aka ImgQ)
  ImgQ curr = cvt(grabber.grab());

  // nested use of operators
  gui["image"] = thresh(abs(last-curr),gui["thresh"]);

  last = curr;
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"-input|-i(2)",init,run).exec();
}
