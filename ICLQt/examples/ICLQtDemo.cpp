#include "ICLWidget.h"
#include "PWCGrabber.h"

#include <QApplication>
#include <QThread>

using namespace icl;

class MyThread : public QThread{
public:
  MyThread():
    widget(new ICLWidget(0)),
    grabber(new PWCGrabber(Size(640,480)))
  {
    widget->setGeometry(200,200,640,480);
    widget->show();
  }
  virtual void run(){
    while(1){
      widget->setImage(grabber->grab());
      widget->update();
    }
  }
  ICLWidget *widget;
  PWCGrabber *grabber;
};


int main(int nArgs, char **ppcArg){
  QApplication a(nArgs,ppcArg);
  MyThread x;
  x.start();
  return a.exec();
}
