#include <ICLDrawWidget.h>
#include <ICLImg.h>
#include <ICLPWCGrabber.h>
#include <QApplication>
#include <ICLQThread>
#include <ICLTimer.h>
#include <math.h>


using namespace icl;
class MyThread : public QThread{
public:
  virtual void run(){
    t=0;
    while(1){

      widget->setImage(grabber->grab());

      widget->lock();
      widget->reset();
      widget->rel();
     
      for(int i=0;i<100;i++){
        x = (1+sin(t/100.+i*i))/2;
        y = (1+cos(t/120.+i))/2;
        
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
      msleep(20);
    }
  }
  PWCGrabber *grabber;
  ICLDrawWidget *widget;
  float x,y,t;
  
};

int main(int nArgs, char **ppcArg){
  QApplication a(nArgs,ppcArg);
  
  MyThread x;
  x.widget = new ICLDrawWidget(0);
  x.widget->setGeometry(200,200,640,480);
  x.widget->show();
  x.grabber = new PWCGrabber(Size(320,240));

  x.start();
  return a.exec();
}
