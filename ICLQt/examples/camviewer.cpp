#include <ICLWidget.h>
#include <PWCGrabber.h>

#include <QApplication>
#include <ProgArg.h>
#include <QThread>

using namespace icl;

class MyThread : public QThread{
public:
  MyThread():
    widget(new ICLWidget(0)),
    grabber(new PWCGrabber(Size(640,480))),
    image(imgNew(translateDepth(pa_subarg("-depth",0,std::string("depth8u"))),
                 Size(640,480),translateFormat(pa_subarg("-format",0,std::string("rgb")))  ))
  {
    widget->setGeometry(200,200,640,480);
    widget->show();
  }
  virtual void run(){
    while(1){
      widget->setImage(grabber->grab(image));
      widget->update();
    }
  }
  ICLWidget *widget;
  PWCGrabber *grabber;
  ImgBase *image;
};


int main(int nArgs, char **ppcArg){
  QApplication a(nArgs,ppcArg);
  pa_init(nArgs,ppcArg,"-format(1) -depth(1)");
  MyThread x;
  x.start();
  return a.exec();
}
