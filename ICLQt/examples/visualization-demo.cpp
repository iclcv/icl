#include <ICLQuick/Common.h>

Grabber *grabber = 0;
ICLDrawWidget *widget = 0;

void run(){
  static float t = 0;
  widget->setImage(grabber->grab());
  
  widget->lock();
  widget->reset();
  widget->rel();
  
  for(int i=0;i<100;i++){
    float x = (1+sin(t/100.+i*i))/2;
    float y = (1+cos(t/120.+i))/2;
    
    widget->color(255,0,0,20);
    widget->line(0,0,x,y);
    widget->color(0,255,0,20);
    widget->line(1,0,x,y);
    widget->color(0,0,255,20);
    widget->line(1,1,x,y);
    widget->color(0,255,255,20);
    widget->line(0,1,x,y);
    widget->fill(255,255,255,128);
    widget->color(255,0,255,128);
    
    widget->fill(255,255,255,20);
    widget->color(255,255,255,150);
    
    
    float w = sin(x+y)/10.;
    float h = cos(w*x+3.2333445)/11.;
    widget->symsize(w/2,h/2);
    widget->sym(x,y,ICLDrawWidget::symPlus);
    widget->sym(x,y,ICLDrawWidget::symRect);
    widget->sym(x,y,ICLDrawWidget::symTriangle);
    widget->sym(x,y,ICLDrawWidget::symCircle);
    widget->sym(x,y,ICLDrawWidget::symCross);
    widget->rect(x-w/2,y-h/2,w,h);
  }
  
  widget->unlock();
  t++;
  widget->update();
  Thread::msleep(5);
}

int main(int n, char **ppc){
  ExecThread x(run);
  QApplication a(n,ppc);
  
  widget = new ICLDrawWidget(0);
  widget->setGeometry(200,200,640,480);
  widget->show();
  grabber = new GenericGrabber("demo","");

  x.run();
  return a.exec();
}
