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


    const Vec pos=Vec(0,0,30,0);
    const Vec norm=Vec(0,0,-1,0); 
    const Vec up=Vec(1,0,0,0); 
    float f=-45;
    float zNear=0.1;
    float zFar=100;
    
    szene = new Szene(Rect(0,0,640,480),Camera(pos,norm,up,f,zNear,zFar));
    
    for(int x=-1;x<2;x++){
      for(int y=-1;y<2;y++){
        for(int z=-1;z<2;z++){
          szene->add(new CubeObject(10*x,10*y,10*z,5));
        }
      }
    }
  }
  virtual void run(){
    Camera &cam = szene->getCam();
    cam.setFocalLength(1.6); /// 1 equals 90° view arc !
    while(1){

      w->lock();
      w->reset();

      szene->transformAllObjs(create_hom_4x4<float>(0.02,0.03,0));  
    
      //      szene->update();
      //szene->render(w);

      for(int x=0;x<3;x++){
        for(int y=0;y<3;y++){
          szene->setViewPort(Rect(x*640/3,y*480/3,640/3,480/3).enlarged(-10));
          szene->update();
          szene->render(w);
        }
      }

      w->unlock();
      w->update();

      szene->setViewPort(Rect(0,0,640,480));
      szene->update();
      image.clear();
      szene->render(&image);
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

