#include <iclQuick.h>
#include <iclPositionTracker.h>
#include <iclThread.h>
#include <iclGUI.h>

#include <iclFPSEstimator.h>
#include <iclArray.h>
#include <iclStringUtils.h>
#include <iclRegionDetector.h>
#include <iclDCGrabber.h>

#include <iclMathematics.h>
#include <iclPoint32f.h>

#include <iclProgArg.h>
#include <iclImgChannel.h>

int error_counter = 0;
int error_frames = 0;
int frame_counter = 0;
int error_counter_save = 0;

void update_error_frames_A(){
  error_counter_save = error_counter;
}

void update_error_frames_B(){
  if(error_counter_save != error_counter){
    error_frames++;
  }
}


struct InputGrabber : public Grabber{
  struct Blob{
    unsigned int x;
    unsigned int y;
    unsigned int r;
    float vx;
    float vy;
    icl8u color[3];
    int id;
    void set_new_id(int new_id){
      if(id != new_id && id != -1){
        error_counter++;
      }
      id = new_id;
    }
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
    b.r +=2;
    {
      int rr = b.r*b.r;
      int h = image.getHeight();
      int w = image.getWidth();
      
      int ystart = iclMax(-b.r,-b.y);
      int yend = iclMin(b.r,h-b.y-1);
      for(int dy = ystart;dy<=yend;++dy){
        int y = dy+b.y;
        int dx = (int)round(::sqrt(rr-dy*dy));
        int xstart = iclMax((unsigned int)(0),b.x-dx);
        int xend = iclMin(b.x+dx,(unsigned int)(w-1));
        for(unsigned int i=0;i<N;++i){
          memline(&c[i](xstart,y),0,xend-xstart);
        }
      }
    }

    b.r -=2;
    {
      int rr = b.r*b.r;
      int h = image.getHeight();
      int w = image.getWidth();
      
      int ystart = iclMax(-b.r,-b.y);
      int yend = iclMin(b.r,h-b.y-1);
      for(int dy = ystart;dy<=yend;++dy){
        int y = dy+b.y;
        int dx = (int)round(::sqrt(rr-dy*dy));
        int xstart = iclMax((unsigned int)(0),b.x-dx);
        int xend = iclMin(b.x+dx,(unsigned int)(w-1));
        for(unsigned int i=0;i<N;++i){
          memline(&c[i](xstart,y),b.color[i],xend-xstart);
        }
      }
    }

    delete [] c;
  }
 
  
  InputGrabber(unsigned int nBlobs=10){
    randomSeed();
    setDesiredSize(Size(640,480));
    image.setSize(getDesiredSize());

    mingap = pa_subarg<int>("-mingap",0,3);
    minr = pa_subarg<int>("-minr",0,10);
    maxr = pa_subarg<int>("-maxr",0,20);
    maxv = pa_subarg<int>("-maxv",0,10);
    maxdv = pa_subarg<int>("-maxdv",0,2);
    maxtrys = 100;
    blobcolor[0] = 255;
    blobcolor[1] = 0;
    blobcolor[2] = 0;

    setBlobCount(nBlobs);
  }
  
  int find_blob(int x, int y){
    for(unsigned int i=0;i<blobs.size();++i){
      Blob &b = blobs[i];
      int dx = b.x-x;
      int dy = b.y-y;
      if(sqrt(dx*dx+dy*dy) < b.r){
        return (int)i;
      }
    }
    return -1;
  }
  
  Blob &get_blob(unsigned int idx){
    return blobs[idx];
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
    b.id = -1;
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
      bool cont = false;
      while(true){
        err = false;
        cont = false;
        b.x += (int)round(b.vx);
        b.y += (int)round(b.vy);
        if(b.x < b.r || b.x >= image.getWidth()-b.r){
          b.vx*=-1;
          cont = true;
        }
        if(b.y < b.r || b.y >= image.getHeight()-b.r){
          b.vy*=-1;
          cont = true;
        }
        if(cont) continue;
        if(!ok(b,i)){
          if(++t > maxtrys){
            err = true;
            break;
          }else{
            b = bSave;
            if(random(1.0) > 0.5){
              b.vx*=-1;
            }else{
              b.vy*=-1;
            }
            continue;
          }
        }else{
          b.vx = clip(b.vx+(float)random(-maxdv,maxdv),-maxv, maxv);
          b.vy = clip(b.vy+(float)random(-maxdv,maxdv),-maxv, maxv);
          break;
        }
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

static inline vec getCenters(const Img8u &image){
  static RegionDetector rd;
  rd.setRestrictions(10,10000,1,255);
  
  const std::vector<icl::Region> &bd = rd.detect(&image);
  
  static vec v;
  v.clear();

  for(unsigned int i=0;i<bd.size();++i){
    Point p = bd[i].getCOG();
    v.push_back(p.x);
    v.push_back(p.y);
  }

  return v;
}



class WorkThread : public Thread{
public:
  WorkThread(){
    if(pa_defined("-dc")){
      vector<DCDevice> devs = DCGrabber::getDeviceList();
      if(!devs.size()){
        ERROR_LOG("no device found");
        exit(-1);
      }
      grabber = new DCGrabber(devs[0]);
    }else{
      grabber = new InputGrabber(pa_subarg<int>("-nblobs",0,30));
    }
    
    grabber->setDesiredSize(Size(640,480));
    grabber->setDesiredFormat(formatGray);
    gui << "draw[@handle=image@minsize=32x24]";
    gui << ( GUI("hbox") 
             << string("slider(0,100,")+pa_subarg<string>("-sleeptime",0,"10") + ")[@handle=Hsl@out=Vsl@label=sleeptime]"
             << "togglebutton(!off,on)[@out=Vlo@label=Show labels]"
           );
    gui.show();

  }
  virtual void run(){

    while(1){
      static ICLDrawWidget *w = *gui.getValue<DrawHandle>("image");

      const ImgBase *image = grabber->grab();
      vec v = getCenters(*(image->asImg<icl8u>()));


      w->setImage(image);



      w->lock();       
      w->reset();

      if(v.size()){
        static PositionTracker<int> pt(pa_subarg<int>("-thresh",0,2));
        
        pt.pushData(v.ptr(),v.size()/2);
        w->color(255,0,0);

        update_error_frames_A();

        for(unsigned int i=0;i<v.size();i+=2){
          
          
          
          w->sym(v[i],v[i+1],ICLDrawWidget::symCross);
          static bool &labelsOnFlag = gui.getValue<bool>("Vlo");

          int curr_x = v[i];
          int curr_y = v[i+1];
          int curr_id = pt.getID(i/2);
          
          int blob_list_idx = ((InputGrabber*)grabber)->find_blob(curr_x,curr_y);
          if(blob_list_idx == -1){
            ERROR_LOG("no blob at location found ???");
          }else{
            InputGrabber::Blob &b=((InputGrabber*)grabber)->get_blob((unsigned int)blob_list_idx);
            b.set_new_id(curr_id);
          }
          
          if(labelsOnFlag){
            static char buf[100];
            w->text(toStr(curr_id,buf),curr_x,curr_y,10);
          }
        }
        update_error_frames_B();
      }

      frame_counter++;

        
      static FPSEstimator fps(10);
      w->color(255,255,255);
      char buf[40];
      sprintf(buf,"blobs:%4d frames:%6d E:frames:%4d all:%4d   ",v.size()/2,frame_counter,error_frames,error_counter);
      w->text(string(buf)+"   "+fps.getFpsString().c_str(),5,5); 
      
      w->unlock();
      w->update();

      static int &sleepTime = gui.getValue<int>("Vsl");
      msleep(sleepTime);
    }
  }
private:
  Grabber *grabber;
};


int main(int n, char  **ppc){
  pa_explain("-dc","use a dc-grabber as video source");
  pa_explain("-nblobs","(int) number of blobs to use");
  pa_explain("-sleeptime","(int) initial sleeptime value");
  pa_explain("-mingap","(int) minimal distance between two blobs");
  pa_explain("-minr","(int) minimal radius of blobs");
  pa_explain("-maxr","(int) maxinal radius of blobs");
  pa_explain("-maxv","(int) maxinal speed of blobs (in pixles per frame)");
  pa_explain("-maxdv","(int) maximal acceleration of blobs");
  pa_explain("-thresh","(int) position tracker threshold for trivial association step");


  pa_init(n,ppc,"-dc -nblobs(1) -sleeptime(1) -mingap(1) -minr(1) -maxr(1) -maxv(1) -maxdv(1) -thresh(1)");
  
  QApplication app(n,ppc);
  WorkThread a;
  a.start();
  
  return app.exec();
}
