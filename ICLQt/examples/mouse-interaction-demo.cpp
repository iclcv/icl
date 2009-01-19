#include <iclCommon.h>
#include <iclTimer.h>

GenericGrabber *grabber;
ICLWidget *widget;


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
} r;

void init(){
  widget = new ICLWidget(0);
  widget->setGeometry(200,200,640,480);
  widget->show();
  grabber = new GenericGrabber;
  widget->add(&r);
}  
void run(){
  widget->setImage(grabber->grab());
  widget->update();
}



int main(int n, char **ppc){
  ExecThread x(run);
  QApplication a(n,ppc);
  
  init();

  x.run();
  return a.exec();
}
