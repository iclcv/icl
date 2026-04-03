#include <ICLQt/Common.h>
#include <ICLCore/Image.h>

GUI gui;
GenericGrabber grabber;

void init(){
   grabber.init(pa("-i"));
   gui << Display().handle("image") << Show();
}
void run(){
   gui["image"] = grabber.grabImage();
}
int main(int n, char **args){
   return ICLApp(n,args,"-input|-i(2)",init,run).exec();
}
