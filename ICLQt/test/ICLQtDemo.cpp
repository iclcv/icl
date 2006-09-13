#include "ICLWidget.h"
#include "Img.h"
#include "PWCGrabber.h"

#include <QApplication>
#include <QThread>
#include <QGridLayout>
#include <QPushButton>
#include <Timer.h>

using namespace icl;

class MyThread : public QThread{
public:
  virtual void run(){
    Img8u image(Size(640,480),3);
    while(1){
      //widget->setImage(grabber->grab());
      widget->setImage(&image);
      widget->update();
      usleep(1000*1000);
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
  //  x.grabber = new PWCGrabber(Size(640,480));
  x.widget->show();

  x.start();
  return a.exec();
}
