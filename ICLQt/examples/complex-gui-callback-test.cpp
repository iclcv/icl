#include <ICLQuick/Common.h>

GUI gui;

void handle_event(const std::string &handle){
  DEBUG_LOG("component " << handle << " was triggered!" );
}

void init(){
  gui << "slider(0,100,50)[@handle=slider@label=slider@out=_1]"
      << "togglebutton(off,on)[@handle=togglebutton@label=toggle button@out=_2]"
      << "button(click me)[@handle=button@label=a button]";
  gui.show();
  
  gui.registerCallback(new GUI::Callback(handle_event),"slider,button,togglebutton");
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"",init).exec();
}

