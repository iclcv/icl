#include <iclCommon.h>

//7#include "iclGUI.h"
//#include <iclDataStore.h>
//#include <iclSize.h>
//#include <iclImg.h>
//#include <iclThread.h>
//#include <QApplication>
//#include <iclQuick.h>
//#include <iclImageHandle.h>
//using namespace icl;

GUI gui;

void run(){
  Img8u image = cvt8u(scale(create("parrot"),0.2));
  ImageHandle *ws[3] = {
    &gui.getValue<ImageHandle>("image1"),
    &gui.getValue<ImageHandle>("image2"),
    &gui.getValue<ImageHandle>("image3")
  };
  ButtonHandle &click = gui.getValue<ButtonHandle>("click");
  while(1){
    for(int i=0;i<3;++i){
      *ws[i] = image;
      ws[i]->update();
    }
    if(click.wasTriggered()){
      std::cout << "button 'click' was triggered!" << std::endl;
    }
    Thread::msleep(50);
  }
}

int main(int n, char **ppc){
  QApplication app(n,ppc);
  
  gui = GUI("hbox");
  gui << "image[@handle=image1@label=image1]"
      << "image[@handle=image2@label=image2]"
      << "image[@handle=image3@label=image3]";
  
  GUI v("vbox[@maxsize=10x1000]");
  v << "slider(-1000,1000,0)[@out=the-int1@maxsize=35x1@label=slider1@minsize=1x2]"
    << "slider(-1000,1000,0)[@out=the-int2@maxsize=35x1@label=slider2@minsize=1x2]"
    << "slider(-1000,1000,0)[@out=the-int3@maxsize=35x1@label=slider3@minsize=1x2]"
    << "combo(entry1,entry2,entry3)[@out=combo@label=the-combobox]"
    << "spinner(-50,100,20)[@out=the-spinner@label=a spin-box]"
    << "button(click me)[@handle=click]";
  gui << v;

  gui.show();

  exec_threaded(run);
  
  return app.exec();
}
