#include <ICLQt/Common.h>

// main GUI
GUI gui;

// process monitor GUI
GUI ps;

// complex callback
void complex_cb(const std::string &handle){
  if(handle == "clickme"){
    std::cout << "'click me' clicked" << std::endl;
  }else if(handle == "moveme"){
    std::cout << "slider value: " << gui["moveme"].as<int>() << std::endl;
  }
}

void init(){
  // only create it
  ps << Ps() << Create();

  // create some nice components
  gui << Button("click me").handle("clickme")
      << Slider(0,100,20).handle("moveme")
      << Button("exit").handle("exit")
      << Button("show ps").handle("show ps")
      << Show();
  
  // register callbacks (on the specific handles)
  gui["exit"].registerCallback(std::terminate);
  gui["clickme"].registerCallback(complex_cb);
  gui["moveme"].registerCallback(complex_cb);
  gui["show ps"].registerCallback(function(ps,&GUI::switchVisibility));
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"",init).exec();
}
