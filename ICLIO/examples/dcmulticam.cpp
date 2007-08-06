#include <iclWidget.h>
#include <iclDrawWidget.h>
#include <iclDCGrabber.h>
#include <QApplication>
#include <iclThread.h>
#include <iclTestImages.h>
#include <iclMutex.h>
using namespace icl;
using namespace std;

Size size(160,120);

class MyThread : public Thread{
public:
  MyThread(){
    std::vector<DCDevice> devs = DCGrabber::getDeviceList();
    if(!devs.size()){
      ERROR_LOG("no dc device found -> exiting!");
      exit(-1);
    }
    w = new ICLWidget(0);
    w->setGeometry(100,100,640,480);
    w->show();
    
    for(unsigned int i=0;i<devs.size();i++){
      gs.push_back(new DCGrabber(devs[i]));
      printf("adding camera %s \n",devs[i].getModelID().c_str());
      gs[gs.size()-1]->setDesiredSize(Size(640,480));
      gs[gs.size()-1]->setDesiredFormat(formatRGB);
      gs[gs.size()-1]->setProperty("format","DC1394_VIDEO_MODE_640x480_MONO8@DC1394_FRAMERATE_30");
    }
    image = new Img8u(Size(devs.size()*640,480),formatRGB);
  }
  
  ~MyThread(){
    m.lock();
    stop();
    msleep(250);
    for(unsigned int i=0;i<gs.size();i++){
      delete gs[i];
    }
    delete w;
    gs.clear();
    m.unlock();
  }
  
  virtual void run(){
    while(1){
      m.lock();
      for(unsigned int i=0;i<gs.size();i++){
        image->setROI(Rect(i*640,0,640,480));
        gs[i]->grab()->deepCopyROI(&image);
      }
      w->setImage(image);
      w->update();
      m.unlock();
      msleep(50);
    }
  }
  int device;
  ICLWidget* w;
  std::vector<DCGrabber*> gs;
  Mutex m;
  ImgBase *image;
};


int main(int nArgs, char **ppcArg){
  QApplication a(nArgs,ppcArg);
  
  MyThread *x=new MyThread;
  x->start();
  a.exec();
  delete x;

  return 0;
  
}
