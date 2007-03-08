#include <ICLWidget.h>
#include <ICLImg.h>
#include <ICLPWCGrabber.h>
#include <QApplication>
#include <QThread>
#include <QGridLayout>
#include <QPushButton>
#include <ICLTimer.h>


using namespace icl;

class MyThread : public QThread{
public:
  MyThread(){
    widget = new ICLWidget(0);
    widget->setGeometry(200,200,640,480);
    widget->show();
    grabber = new PWCGrabber(Size(640,480));
  }
  virtual void run(){
    while(1){
      widget->setImage(grabber->grab());
      widget->update();
    }
  }
  PWCGrabber *grabber;
  ICLWidget *widget;
};


class Receiver : public MouseInteractionReceiver{
public:
  virtual void processMouseInteraction(MouseInteractionInfo *info){
    printf("mouse Event at image(%d,%d)  type = ",info->imageX, info->imageY);
    switch(info->type){
      case MouseInteractionInfo::dragEvent: 
        printf("dragEvent \n");
        break;
      case MouseInteractionInfo::pressEvent:
        printf("pressEvent ");
        if(info->color.size() > 0){
          printf("clicked color was %f,%f,%f \n",info->color[0],info->color[1], info->color[2]);
        }else{
          printf("no color \n");
        }
        break;
      case MouseInteractionInfo::releaseEvent:
        printf("releaseEvent \n");
        break;
      case MouseInteractionInfo::moveEvent:
        printf("moveEvent \n");
        break;
      case MouseInteractionInfo::enterEvent:
        printf("enterEvent \n");
        break;
      case MouseInteractionInfo::leaveEvent:
        printf("leaveEvent \n");
        break;
    }
  } 
};

int main(int nArgs, char **ppcArg){
  QApplication a(nArgs,ppcArg);
  
  MyThread x;
  Receiver r;                                   
  
  QObject::connect(x.widget,SIGNAL(mouseEvent(MouseInteractionInfo*)),
                   &r,SLOT(mouseInteraction(MouseInteractionInfo*)));

  x.start();
  return a.exec();
}
