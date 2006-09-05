#include "ICLWidget.h"
#include "Img.h"
#include "PWCGrabber.h"

#include <QApplication>
#include <QThread>

using namespace icl;
class MyThread : public QThread{
public:
  virtual void run(){
    Img8u i(Size(320,240),3);
    i.clear(0,255);
    while(1){
      //      widget->setImage(grabber->grab());
      widget->setImage(&i);
      widget->update();
      msleep(200);
    }
  }
  //  PWCGrabber *grabber;
  ICLWidget *widget;
};

int main(int nArgs, char **ppcArg){
  QApplication a(nArgs,ppcArg);
  
  MyThread x;
  x.widget = new ICLWidget(0);
  x.widget->setGeometry(200,200,640,480);
  x.widget->show();
  //  x.grabber = new PWCGrabber(Size(320,240));

  x.start();
  return a.exec();
}
