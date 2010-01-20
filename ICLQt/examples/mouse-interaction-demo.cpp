#include <ICLQuick/Common.h>
#include <iterator>

GenericGrabber *grabber;
ICLWidget *widget;


class Mouse : public MouseHandler{
public:
  virtual void process(const MouseEvent &event){
    std::cout << "image location: " << event.getPos() << std::endl;    
    std::cout << "widget location: " << event.getWidgetPos() << std::endl;
    
    std::string eventNames[] = {"MoveEvent","DragEvent","PressEvent","ReleaseEvent", "EnterEvent","LeaveEvent"};
    std::cout << "type: " << eventNames[event.getType()] << std::endl;
    
    if(event.isPressEvent()){
      if(event.getColor().size()){
        std::cout << "color:";
        std::copy(event.getColor().begin(),event.getColor().end(),std::ostream_iterator<icl64f>(std::cout,","));
        std::cout << endl;
      }else{
        std::cout << "no color here!" << std::endl;
      }
    }
  } 
} mouse;

void init(){
  widget = new ICLWidget(0);
  widget->setGeometry(200,200,640,480);
  widget->show();
  grabber = new GenericGrabber;
  widget->install(&mouse);
}  
void run(){
  widget->setImage(grabber->grab());
  widget->update();
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"",init,run).exec();
}
