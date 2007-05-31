#include <iclDrawWidget.h>
#include <iclImg.h>
#include <iclImgBase.h>
#include <iclPWCGrabber.h>
#include <iclCC.h>
#include <iclImgChannel.h>
#include <QApplication>
#include <QThread>
#include <QGridLayout>
#include <QPushButton>
#include <iclTimer.h>

int w=320;
int h=240;

using namespace icl;
using namespace std;
class CamThread : public QThread {
public:
  CamThread():widget(new ICLDrawWidget(0)),
              widget2(new ICLDrawWidget(0)),
              grabber2(new PWCGrabber(Size(w,h),30,0)),
              grabber(new PWCGrabber(Size(w,h),30,1)) {
    //    grabber->setDesiredSize(Size(w,h));
    //grabber2->setDesiredSize(Size(w,h));
    widget->setGeometry(200,200,w,h);
    widget2->setGeometry(200+w+50,200,w,h);
    widget->show();
    widget2->show();
  }
  
  virtual void run(){
    while(1){
      const ImgBase *grabbedImage = grabber->grab();
      const ImgBase *grabbedImage2 = grabber2->grab();

      widget->setImage(grabbedImage);
      widget->update();
      widget2->setImage(grabbedImage2);
      widget2->update();
    }
  }
  
private:
  ICLDrawWidget *widget;
  ICLDrawWidget *widget2;

  PWCGrabber *grabber2;
  PWCGrabber *grabber;
};



//int main(int nArgs, char **ppcArg){
int main(int nArgs, char *ppcArg[]){
  QApplication a(nArgs,ppcArg);
  CamThread x;
  x.start();
  return a.exec();
}
