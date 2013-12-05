#include <ICLQt/Common.h>
#include <ICLIO/GenericImageOutput.h>

GUI gui;
GenericGrabber grabber;
GenericImageOutput out;

void init(){
   grabber.init(pa("-i"));
   out.init(pa("-o"));
   gui << Image().handle("image") << Show();
}
void run(){
  const ImgBase *image = grabber.grab();
  gui["image"] = image;
  out.send(image);
}
int main(int n, char **args){
  return ICLApp(n,args,"-input|-i(2) -output|-o(2)",init,run).exec();
}
