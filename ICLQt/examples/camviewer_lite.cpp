#include <iclWidget.h>
#include <iclDrawWidget.h>
#include <iclPWCGrabber.h>
#include <QApplication>
#include <QThread>
#include <iclTestImages.h>
using namespace icl;
using namespace std;

Size size(160,120);

class MyThread : public QThread{
public:
  MyThread(int device){
    this->device = device;
    widget = new ICLWidget(0);
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
    while(1){
      widget->setImage(g.grab());
      widget->update();
    }
  }
  int device;
  ICLWidget *widget;
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
