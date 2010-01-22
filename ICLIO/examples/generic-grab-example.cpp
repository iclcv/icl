#include <ICLQuick/Common.h>

GUI gui("vbox");
GenericGrabber *grabber = 0;
std::string params[] = {"","0","0","*","*.ppm",""};
Mutex mutex;

void change_grabber(){
  Mutex::Locker l(mutex);
  gui_ComboHandle(source);

  std::string newType = source.getSelectedItem();
  int idx = source.getSelectedIndex();

  if(!grabber || grabber->getType() != newType){
    ICL_DELETE( grabber );
    try{
      grabber = new GenericGrabber(newType,newType+"="+params[idx],false);
    }catch(...){}
    if(grabber->isNull()){
      ICL_DELETE( grabber );
    }
  }
}

void init(){
  gui << "image[@minsize=32x24@handle=image]" 
      << "combo(null,pwc,dc,unicap,file,demo)[@label=source@out=_@handle=source]";
  
  gui.show();
  
  gui.registerCallback(new GUI::Callback(change_grabber),"source");

  if(pa_defined("-input")){
    grabber = new GenericGrabber(FROM_PROGARG("-input"));
  }
}

void run(){
  Mutex::Locker l(mutex);
  
  if(grabber){
    gui["image"] = grabber->grab();
    gui["image"].update();
  }else{
    Thread::msleep(20);
  }
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"-input(2)",init,run).exec();
}
