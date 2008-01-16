#include <iclQuick.h>
#include <iclPositionTracker.h>
#include <iclThread.h>
#include <iclGUI.h>

#include <iclFPSEstimator.h>
#include <iclArray.h>
#include <iclStringUtils.h>
#include <iclImgRegionDetector.h>
#include <iclDCGrabber.h>

#include <iclMathematics.h>
#include <iclPoint32f.h>

#include <iclProgArg.h>
#include <iclImgChannel.h>

struct InputGrabber : public Grabber{
  struct Blob{
    unsigned int x;
    unsigned int y;
    unsigned int r;
    float vx;
    float vy;
    icl8u color[3];
    void show(int idx=-1){
      printf("Blob idx=%d pos=(%d,%d) r=%d vx=%f vy=%f color=(%d,%d,%d)\n",
             idx,x,y,r,vx,vy,color[0],color[1],color[2]);
    }
  };
    
  static inline unsigned int distance(int x1, int y1, int x2, int y2){
    return (unsigned int)round(::sqrt(pow(float(x1-x2),2)+pow(float(y1-y2),2)));
  }
  static inline void memline(icl8u *p, icl8u v, int len){
    for(icl8u *c = p+len-1; c >= p; --c){
      *c = v;
    }
  }
  
  template<unsigned int N>
  static inline void draw_blob(Img8u &image, Blob &b){
    ImgChannel8u *c =  new ImgChannel8u[N];
    for(unsigned int i=0;i<N;++i){
      c[i] = pickChannel(&image,i);
    }

    int rr = b.r*b.r;
    int h = image.getHeight();
    int w = image.getWidth();
    
    int ystart = iclMax(-b.r,-b.y);
    int yend = iclMin(b.r,h-b.y);
    for(int dy = ystart;dy<=yend;++dy){
      int y = dy+b.y;
      int dx = (int)round(::sqrt(rr-dy*dy));
      int xstart = iclMax((unsigned int)(0),b.x-dx);
      int xend = iclMin(b.x+dx,(unsigned int)(w-1));
      for(unsigned int i=0;i<N;++i){
        memline(&c[i](xstart,y),b.color[i],xend-xstart);
      }
    }
    delete [] c;
  }
 
  
  InputGrabber(unsigned int nBlobs=10){
    randomSeed();
    setDesiredSize(Size(640,480));
    image.setSize(getDesiredSize());

    mingap = 3;
    minr = 3;
    maxr = 10;
    maxv = 1;
    maxdv = 1;
    maxtrys = 500;
    blobcolor[0] = 255;
    blobcolor[1] = 0;
    blobcolor[2] = 0;

    setBlobCount(nBlobs);
  }
  bool ok(const Blob &b, int idx_b=-1){
    Rect imageRect = image.getImageRect();
    for(unsigned int i=0;i<blobs.size();++i){
      if((int)i == idx_b){
        continue;
      }
      if(distance(b.x,b.y,blobs[i].x,blobs[i].y) < mingap+b.r+blobs[i].r ||
         !imageRect.enlarged(-b.r).contains(b.x,b.y) ){
        return false;
      }
    }
    return true;
  }

  Blob new_blob(){
    Blob b;
    b.color[0] = blobcolor[0];
    b.color[1] = blobcolor[1];
    b.color[2] = blobcolor[2];
    unsigned int t = 0;
    do{
      b.x = random((unsigned int)image.getWidth());
      b.y = random((unsigned int)image.getHeight());
      b.r = minr+random(maxr-minr);
      b.vx = random(maxv);
      b.vy = random(maxv);     
      if(++t >= maxtrys){
        throw(ICLException("Max trys reached!"));
      }   

    }while(!ok(b));
    return b;
  }

  void setBlobCount(unsigned int nBlobs){
    if(nBlobs < blobs.size()){
      blobs.resize(nBlobs);
    }else{
      try{
        for(unsigned int i=blobs.size();i<nBlobs;++i){
          blobs.push_back(new_blob());
        }
      }catch(const ICLException &ex){
        ERROR_LOG("no place for " << nBlobs << " blobs break at " << blobs.size() << " !");
      }
    }
  }
  
  void iterate(){
    for(unsigned int i=0;i<blobs.size(); ++i){
      Blob &b = blobs[i];
      Blob bSave = b;
      unsigned int t=0;
      bool err = false;
      do{
        b=bSave;
        b.x += (int)round(b.vx);
        b.y += (int)round(b.vy);
        if(b.x < b.r || b.x > image.getWidth()-b.r) b.vx*=-1;
        if(b.y < b.r || b.y > image.getHeight()-b.r) b.vy*=-1;
        if(++t > maxtrys){
          err = true;
        }
      }while(!err && !ok(b,i));
      if(err){
        b=bSave;
      }else{
        b.vx += random(-maxdv,maxdv);
        b.vy += random(-maxdv,maxdv);
        b.vx = clip(b.vx,-maxv, maxv);
        b.vy = clip(b.vy,-maxv, maxv);
      }
    }
  }
  
  virtual const ImgBase *grab(ImgBase **dst=0){
    ICLASSERT_RETURN_VAL(!dst,0);
    ICLASSERT_RETURN_VAL(getDesiredDepth() == depth8u,0);

    image.setSize(getDesiredSize());
    image.setFormat(getDesiredFormat());
    
    image.clear();
    
    iterate();
    
    if(image.getChannels() == 1){
      for(unsigned int i=0;i<blobs.size();++i){
        draw_blob<1>(image,blobs[i]);
      }
    }else if(image.getChannels() == 3){
      for(unsigned int i=0;i<blobs.size();++i){
        draw_blob<3>(image,blobs[i]);
      }
    }else{
      ERROR_LOG("invalid channel count! (allowed is 1 or 3 but found " << image.getChannels() << ")");
    }
    
    return &image;    
  }
  
private:
  Img8u image;
  vector<Blob> blobs;
  unsigned int mingap;
  unsigned int minr;
  unsigned int maxr;
  float maxv;
  float maxdv;
  unsigned int maxtrys;
  icl8u blobcolor[3];
};




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
    if(pa_defined("-dc")){
      vector<DCDevice> devs = DCGrabber::getDeviceList();
      if(!devs.size()){
        ERROR_LOG("no device found");
        exit(-1);
      }
      grabber = new DCGrabber(devs[0]);
    }else{
      grabber = new InputGrabber(30);
    }
    
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
  Grabber *grabber;
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
        pt.pushData(v.ptr(),v.size()/2);

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
  pa_explain("-dc","use a dc-grabber as video source");
  pa_init(n,ppc,"-dc");
  
  QApplication app(n,ppc);
  WorkThreadA a;
  WorkThreadB b;
  a.start();
  b.start();
  
  return app.exec();
}
