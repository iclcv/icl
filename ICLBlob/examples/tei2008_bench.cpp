#include <iclQuick.h>
#include <iclPositionTracker.h>
#include <iclThread.h>
#include <iclGUI.h>

#include <iclFPSEstimator.h>
#include <iclArray.h>
#include <iclStringUtils.h>
#include <iclImgRegionDetector.h>
#include <iclDCGrabber.h>

typedef Array<int> vec;

GUI gui;

static inline vec getCenters(const Img8u &image, Img8u *dstRet){
  static ImgRegionDetector rd;
  rd.setRestrictions(10,10000,1,255);
  
  static Img8u dst(image.getSize(),formatGray);
  
  int w = image.getWidth();
  int h = image.getHeight();
  const int dim = w*h;
  const icl8u *r = image.getData(0);
  const icl8u *g = image.getData(1);
  const icl8u *b = image.getData(2);
  
  
  icl8u *d = dst.getData(0);
  
  for(int i=0;i<dim;++i){
    d[i] = 255 * ( (r[i] > 100) && (g[i] < 100) && (b[i] < 100) );
  }
  
  const std::vector< BlobData > &bd = rd.detect(&dst);
  
  static vec v;
  v.clear();

  for(unsigned int i=0;i<bd.size();++i){
    Point p = bd[i].getCOG();
    v.push_back(p.x);
    v.push_back(p.y);
  }
  if(dstRet){
    dst.deepCopy(dstRet);
  }
  return v;
}

Mutex datamutex;
vec DATA;

Mutex drawhandlemutex;
static DrawHandle *dh;

class WorkThreadA : public Thread{
public:
  WorkThreadA(){
    vector<DCDevice> devs = DCGrabber::getDeviceList();
    if(!devs.size()){
      ERROR_LOG("no device found");
      exit(-1);
    }
    grabber = new DCGrabber(devs[0]);
    grabber->setDesiredSize(Size(640,480));
    gui << "draw[@handle=image@minsize=32x24]";
    gui << ( GUI("hbox") 
             << "label()[@handle=l1@label=vision fps]"
             << "label()[@handle=l2@label=linass fps]"
             << "label()[@handle=l3@label=center count]"
             << "slider(0,100,10)[@handle=Hsl@out=Vsl@label=sleeptime]"
             << "togglebutton(!off,on)[@out=Vlo@label=Show labels]"
           );
    gui.show();
    dh = &gui.getValue<DrawHandle>("image");
  }
  virtual void run(){

    while(1){
      static Img8u dst;
      static LabelHandle &l = gui.getValue<LabelHandle>("l1");
      static FPSEstimator fps(10);
      l = fps.getFpsString();


      const ImgBase *image = grabber->grab();

      vec v = getCenters(*(image->asImg<icl8u>()),&dst);

      datamutex.lock();
      DATA = v;
      datamutex.unlock();

      drawhandlemutex.lock();
      (*dh) = &dst;
      static LabelHandle &lN = gui.getValue<LabelHandle>("l3");
      lN = (int)v.size();
      drawhandlemutex.unlock();

      static int &sleepTime = gui.getValue<int>("Vsl");
      msleep(sleepTime);
    }
  }
private:
  DCGrabber *grabber;
};

class WorkThreadB : public Thread{
public:
  virtual void run(){
    PositionTracker<int> pt;
    while(1){
      static LabelHandle &l = gui.getValue<LabelHandle>("l2");
      static FPSEstimator fps(10);
      l = fps.getFpsString();

      datamutex.lock();
      vec v = DATA;
      datamutex.unlock();

      drawhandlemutex.lock();
      ICLDrawWidget *w = **dh;
      w->lock();       
      w->reset();
      if(v.size()){
        pt.pushData(v.data(),v.size()/2);

        w->color(255,0,0);
        for(unsigned int i=0;i<v.size();i+=2){
          w->sym(v[i],v[i+1],ICLDrawWidget::symCross);
          static bool &labelsOnFlag = gui.getValue<bool>("Vlo");
          if(labelsOnFlag){
            static char buf[100];
            w->text(toStr(pt.getID(i/2),buf),v[i],v[i+1],10);
          }
        }
      }
      w->unlock();
      drawhandlemutex.unlock();
      w->update();

      static int &sleepTime = gui.getValue<int>("Vsl");
      msleep(sleepTime);
    }
  }
};


int main(int n, char  **ppc){
  QApplication app(n,ppc);
  WorkThreadA a;
  WorkThreadB b;
  a.start();
  b.start();
  
  return app.exec();
}
