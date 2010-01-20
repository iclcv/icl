#include <ICLQuick/Common.h>

GUI gui;
ICLDrawWidget *w = 0;
void init(){
  gui << "draw[@handle=image]";
  gui.show();
  
  w = *gui.getValue<DrawHandle>("image");
  
  ImgQ x = scale(create("parrot"),100,100);
  w->addSpecialButton("im",&x,&ICLWidget::captureCurrentImage);
  
  ImgQ k = zeros(100,100,4);
  color(255,0,0,255);
  font(50);
  text(k,2,2,"FB");
  for(int x=0;x<100;++x){ 
    for(int y=0;y<100;++y){
      k(x,y,3) = k(x,y,0);
    }
  }
  
  w->addSpecialButton("fb",&k,&ICLWidget::captureCurrentFrameBuffer);

  
}

void run(){
  static GenericGrabber grabber(FROM_PROGARG("-input"));
  gui["image"] = grabber.grab();
  gui["image"].update();
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"-input(2)",init,run).exec();
}
