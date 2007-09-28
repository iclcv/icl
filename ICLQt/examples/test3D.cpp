#include <iclWidget.h>
#include <iclDrawWidget3D.h>
#include <iclPWCGrabber.h>
#include <QApplication>
#include <QThread>
#include <iclTestImages.h>
using namespace icl;
using namespace std;

Size size(640,480);


class MyThread : public QThread{
public:
  MyThread(int device){
    this->device = device;
    widget = new ICLDrawWidget3D(0);
    widget->setGeometry(200,200,640,480);
    widget->show();
    start();
  }
  ~MyThread(){
    exit();
    msleep(250);
  }
  
  virtual void run(){
    PWCGrabber g(size,24,device);
    g.setDesiredSize(size);
    
    static float rz = 2;    
    static float ry = 1;
    while(1){
      widget->setImage(g.grab());
      
      widget->lock();
      widget->reset3D();
      
    
      //   widget->matrixmode3D(false);
      widget->rotate3D(0,ry,rz);
      
      rz += 0.4;
      ry += 0.8;

      widget->color3D(1, 0, 0, 1);
      widget->cube3D(0,0,0,0.2);
      
      // 2D Stuff
      widget->reset();
      widget->color(255,0,0,100);
      widget->fill(255,0,0,50);
      widget->rect(100,100,100,100);
      widget->rel();
      widget->rect(0.1,0.1,0.8,0.8);
      widget->abs();
      widget->color(0,100,255);
      widget->fill(0,100,255);
      widget->text("This is a text",300,100,80,20);
      
      widget->unlock();
      
      
      
      widget->update();
    }
  }
  int device;
  ICLDrawWidget3D *widget;
};


int main(int nArgs, char **ppcArg){
  int device=0;
  if(nArgs == 2){
    device = atoi(ppcArg[1]);
  }
  QApplication a(nArgs,ppcArg);
  MyThread x(device);
  return a.exec();
}
