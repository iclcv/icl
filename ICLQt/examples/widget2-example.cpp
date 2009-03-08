#include <iclCommon.h>

#include <iclWidget2.h>


ICLWidget2 *w = 0;

void run(){
  //  static GenericGrabber g("file","/home/celbrech/images/chrissi-heller.jpg*");
  static ImgQ image = scale(create("parrot"),0.5);
  w->setImage(&image);
  w->update();
  Thread::msleep(50);
}

int main(int n, char **ppc){
  ExecThread t(run);
  QApplication app(n,ppc);

  w = new ICLWidget2(0);
  w->setGeometry(QRect(100,100,640,480));
  w->show();

  t.run();
  
  return app.exec();
  
}
