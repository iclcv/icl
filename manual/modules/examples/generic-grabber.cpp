#include <ICLQt/Common.h>

GUI gui;
GenericGrabber grabber;

void init(){
   grabber.init(pa("-i"));
   gui << Image().handle("image") << Show();
}
void run(){
   gui["image"] = grabber.grab();
}
int main(int n, char **args){
   return ICLApp(n,args,"-input|-i(2)",init,run).exec();
}
