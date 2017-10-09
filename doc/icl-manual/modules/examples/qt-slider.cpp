#include <ICLQt/Common.h>

// top level GUI
GUI gui;

void init(){
  // create the GUI by streaming components
  gui << qt::Slider(0,100,50).label("a slider")
      << qt::Show();
}

int main(int n, char**ppc){
  return qt::ICLApp(n,ppc,"",init).exec();
}
