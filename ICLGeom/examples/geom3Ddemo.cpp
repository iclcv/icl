#include <iclQuick.h>
#include <iclGeom.h>

#include <iclSzene.h>
#include <iclCamera.h>
#include <iclCubeObject.h>

#include <iclDrawWidget.h>
#include <QApplication>
#include <iclThread.h>

using namespace icl;


class MyThread : public Thread{
public:
  MyThread(){
    w = new ICLDrawWidget(0);
    w->setGeometry(QRect(100,100,640,480));
    w->show();

    w2 = new ICLDrawWidget(0);
    w2->setGeometry(QRect(800,100,320,240));
    w2->show();
    
    image = ImgQ(Size(640,480),formatRGB);
    w->setImage(&image);
    w2->setImage(&image);

    szene = new Szene(Size(640,480));
    
    szene->add(new CubeObject(0,0,0,3));

  }
  virtual void run(){
    Camera &cam = szene->getCam();
    cam.setFocalLength(4); /// 1 equals 90° view arc !
    while(1){

      w->lock();
      w->reset();
       
      szene->update();
      szene->transformAllObjs(Mat::rot(0.02,0.03,0));
      szene->render(w);
      image.clear();
      szene->render(&image);
      w->unlock();
      w->update();
      
      w2->setImage(&image);
      w2->update();
      msleep(50);
      
    }
  }
  ImgQ image;
  Szene *szene;
  ICLDrawWidget *w,*w2;
};


int main(int n, char**ppc){
  QApplication app(n,ppc);
  
  MyThread t;
  t.start();

  return app.exec();
}

