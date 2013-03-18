
#include <ICLQt/Common.h>

GUI gui;
void init(){
  gui << Slider(0,100,50).out("foo")
      << Label().handle("bar")
      << Show();
}

void run(){
  // create a static reference to the  
  static int &sliderValue = gui.get<int>("foo");
  
  gui["bar"] = sliderValue;
}

int main(int n, char **args){
  return ICLApp(n,args,"",init,run).exec();
}
