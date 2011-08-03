#include <ICLQuick/Common.h>
#include <ICLUtils/Configurable.h>

GUI gui("hbox");

struct MyConfigurable : public Configurable{
  MyConfigurable(){
    addProperty("a range with slider","range:slider","[-1,1]",0);
    addProperty("a volatile slider","range:slider","[-1,1]",0,100);
    addProperty("a range with spinbox","range:spinbox","[0,255]:1",0);
    addProperty("a float value","float","[-1e38,1e38]",0);
    addProperty("an int value","int","[-100,100]",0);
    addProperty("a string ","string","10","test");
    addProperty("a command","command","");
    addProperty("menu.a menu","menu","1,2,3,4,hello,test","test");
    addProperty("some flag","flag","",true);
    addProperty("an info","info","","",0);
  }
} cfg;

void init(){
  cfg.setConfigurableID("cfg");
  gui << "prop(cfg)" << "prop(cfg)" << "!show";
}
void run(){
  Thread::msleep(100);
  cfg.setPropertyValue("an info",str(Time::now()));
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"",init,run).exec();
}
