#include "iclGUI.h"
#include <iclGUIDataStore.h>
#include <iclSize.h>
#include <iclImg.h>
#include <iclThread.h>
#include <QApplication>
#include <iclQuick.h>

using namespace icl;

class MyThread: public Thread{
public:
  MyThread(GUI *gui){
    this->gui = gui;
    n=0;
    start();
  }
  virtual void run(){
    gui->waitForCreation();
    Img8u image = cvt8u(scale(create("parrot"),0.2));
    while(1){
      gui->lockData();
      ICLWidget *w1 = gui->getValue<ICLWidget*>("image1");
      w1->setImage(&image);
      w1->update();

      ICLWidget *w2 = gui->getValue<ICLWidget*>("image2");
      w2->setImage(&image);
      w2->update();

      ICLWidget *w3 = gui->getValue<ICLWidget*>("image3");
      w3->setImage(&image);
      w3->update();
      gui->unlockData();
      msleep(50);
    }
  }
private:
  int n;
  GUI *gui;
};

int main(int n, char **ppc){
  QApplication app(n,ppc);
  
  GUI g("hbox");
  g << "image[@inp=image1@label=image 1]";
  g << "image[@inp=image2@label=image 2]";
  g << "image[@inp=image3@label=image 3]";
  GUI v("vbox[@maxsize=10x1000]");
  v << "slider(-1000,1000,0)[@out=the-int1@maxsize=5x1@label=slider1]";
  v << "slider(-1000,1000,0)[@out=the-int2@maxsize=5x1@label=slider2]";
  v << "slider(-1000,1000,0)[@out=the-int3@maxsize=5x1@label=slider3]";
  v << "combo(entry1,entry2,entry3)[@out=combo@label=the-combobox]";
  v << "spinner(-50,100,20)[@out=the-spinner@label=a spin-box]";
  g << v;
  //  g << "image[@inp=image2@size=4x4]";
  //g << "image[@inp=3mage2@size=4x4]";
  
  g.show();
  
  MyThread t(&g);
  
  return app.exec();
}
