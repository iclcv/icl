#include <iclWidget.h>
#include <iclDrawWidget3D.h>
#include <iclFileGrabber.h>
#include <iclDCGrabber.h>
#include <QApplication>
#include <QThread>
#include <iclTestImages.h>
#include <iclFPSEstimator.h>
using namespace icl;
using namespace std;

Size size(320,240);


class MyThread : public QThread{
public:
  MyThread(int device){
    this->device = device;
    widget = new ICLDrawWidget3D(0);
    widget->setGeometry(200,200,640,480);
    widget->show();
    start();
  }
  ~MyThread(){
    exit();
    msleep(250);
  }
  
  virtual void run(){
    Grabber *g=0;
    vector<DCDevice> devs = DCGrabber::getDeviceList();
    if(!devs.size()){
      g = new FileGrabber("images/*.ppm");
    }else{
      g = new DCGrabber(devs[0]);
      g->setProperty("format","DC1394_VIDEO_MODE_640x480_MONO8@DC1394_FRAMERATE_60");
    }
    g->setDesiredSize(size);
    
    static float rz = 2;    
    static float ry = 1;
    while(1){
      const ImgBase *image = g->grab();
      widget->setImage(image);
      
      widget->lock();
      widget->reset3D();
      
    
      //   widget->matrixmode3D(false);
      widget->rotate3D(0,ry,rz);
      
      rz += 0.4;
      ry += 0.8;

      widget->color3D(1, 1, 1, 1);
      static FileGrabber gFile("images/*.ppm");
      widget->imagecube3D(0,0,0,0.5,gFile.grab());
      // widget->scale3D(0.3,0.3,0.3);
      // widget->translate3D(-0.5,-0.5,0);
      //widget->image3D(0,0,0,640.0/480.0,0,0,0,1,0,image);
      
      
      // 2D Stuff
      widget->reset();
      widget->color(255,0,0,100);
      widget->fill(255,0,0,50);
      widget->rel();
      widget->rect(0.01,0.01,0.5,0.05);
      widget->color(255,255,255,200);
      widget->fill(255,255,255,200);
      static FPSEstimator fps(10);
    
      widget->text(fps.getFpsString(),0.02,0.02,0.2,0.03);

      widget->unlock();
      
      
      
      widget->update();
      msleep(10);
    }
  }
  int device;
  ICLDrawWidget3D *widget;
};


int main(int nArgs, char **ppcArg){
  int device=0;
  if(nArgs == 2){
    device = atoi(ppcArg[1]);
  }
  QApplication a(nArgs,ppcArg);
  MyThread x(device);
  return a.exec();
}
