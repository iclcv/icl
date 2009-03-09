#include <iclCommon.h>

#include <iclWidget2.h>


ICLWidget2 *w = 0;

void run(){
  static GenericGrabber g("pwc","pwc=0");
  w->setImage(g.grab());
  w->update();
  Thread::msleep(50);
}

int main(int n, char **ppc){
  pa_init(n,ppc,"-loop");
  if(pa_defined("-loop")){
    ExecThread t(run);
    QApplication app(n,ppc);
    
    w = new ICLWidget2(0);
    w->setGeometry(QRect(100,100,640,480));
    w->show();
    
    t.run();
    
    return app.exec();
  }else{
    QApplication app(n,ppc);
    
    w = new ICLWidget2(0);
    w->setGeometry(QRect(100,100,640,480));
    w->show();
    
    run();
    
    return app.exec();
  }
  
}
