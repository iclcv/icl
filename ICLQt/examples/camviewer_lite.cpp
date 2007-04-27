#include <iclWidget.h>
#include <iclDrawWidget.h>
#include <iclPWCGrabber.h>
#include <QApplication>
#include <QThread>
#include <iclTestImages.h>
using namespace icl;
using namespace std;

class MyThread : public QThread{
public:
  MyThread(){
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
    PWCGrabber g(Size(320,240));
    g.setDesiredSize(Size(800,600));
    while(1){
      widget->setImage(g.grab());
      widget->update();
    }
  }
  ICLWidget *widget;
};


int main(int nArgs, char **ppcArg){
  QApplication a(nArgs,ppcArg);
  MyThread x;
  return a.exec();
}
