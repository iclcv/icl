#include <iclCommon.h>
#include <iclVideoGrabber.h>

GUI gui;
std::string fileName;
VideoGrabber *g = 0;
bool disableNextUpdate = false;
bool mouseInWindow = false;

void set_stream_pos(){
  if(!disableNextUpdate){
    gui_int(posVal);
    g->setProperty("stream-pos",str(posVal));
  }
  disableNextUpdate = false;
}

void init(){
  gui << "draw[@minsize=32x24@handle=image]" 
      << ( GUI("hbox[@maxsize=1000x3]") 
           << "fps(100)[@handle=fps@maxsize=5x2]" 
           << "slider(0,65535,0)[@label=stream position@out=posVal@handle=pos@maxsize=1000x2]"
           << "togglebutton(play,pause)[@out=pause@maxsize=6x2]"
          );
  
  gui.show();
  g = new VideoGrabber(fileName);
  g->setIgnoreDesiredParams(true);
  
  gui.registerCallback(new GUI::Callback(set_stream_pos),"pos");
}

void run(){
  gui_SliderHandle(pos);
  gui_DrawHandle(image);
  gui_bool(pause);
  
  while(pause){
    Thread::msleep(100);
  }
  
  image = g->grab();
  image.update();

  gui["fps"].update();

  int p = parse<int>(g->getValue("stream-pos"));
  disableNextUpdate = true;
  if(pos.getValue() != p) pos.setValue(p);
  
  Thread::msleep(1);
}

int main(int n,char **ppc){
  if(n != 2){
    std::cerr << "usage: icl-video-player <Input File Name>" << std::endl;
    return -1;
  }
  fileName = ppc[1];
  return ICLApplication(n,ppc,"",init,run).exec();
}
