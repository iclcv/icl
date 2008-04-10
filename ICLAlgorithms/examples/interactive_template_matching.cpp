#include <iclQt.h>
#include <iclUsefulFunctions.h>
#include <iclQuick.h>
#include <iclGenericGrabber.h>
#include <iclRegionDetector.h>
#include <iclViewBasedTemplateMatcher.h>

Size imageSize(640,480);
GUI gui("hbox");
Img8u currTempl;
Img8u templMask(Size(1,1),1);

Img8u currImage;
Img8u imageMask = cvt8u(ones(imageSize.width,imageSize.height,1)*255);

Mutex mutex;
bool dragging = false;
Rect currRect = Rect(Point::null,imageSize).enlarged(-imageSize.width/2);


void init(){
  gui << "draw()[@label=image@minsize=32x24@handle=image]";
  gui << ( GUI("vbox") 
           << "draw()[@label=template@minsize=10x6@handle=templ]"
           << "draw()[@label=buffer@minsize=10x6@handle=buf]"
          );
           

  GUI controls("vbox[@minsize=7x7]");
  controls << "fslider(0,1,0.9)[@handle=significance-handle@label=significance@out=significance]";
  controls << "fps(10)[@handle=fps@minsize=5x5]";
  controls << "togglebutton(no masks, with masks)[@out=use-masks]";
  controls << "togglebutton(dont clip buffers,clip buffers)[@out=clip-buffers]";
  controls << "togglebutton(square distance,norm. cross corr)[@out=mode]";
  
  gui << controls;
   
  gui.show();

  static struct X : public MouseInteractionReceiver {
    // {{{ open

    virtual void processMouseInteraction(MouseInteractionInfo *info){
      if(info->type == MouseInteractionInfo::pressEvent){
        currRect.x = info->imageX;
        currRect.y = info->imageY;
        currRect.width = 1;
        currRect.height = 1;
        dragging = true;
      }else if(info->type == MouseInteractionInfo::releaseEvent){
        Rect r = currRect.normalized() & Rect(Point::null,imageSize);
        Mutex::Locker l(mutex);
        currImage.setROI(r);
        currTempl.setSize(r.size());
        currImage.deepCopyROI(&currTempl);
        templMask.setSize(currTempl.getSize());
        templMask.clear(-1,255,false);
        dragging =false;
      }else if(info->type == MouseInteractionInfo::dragEvent){
        currRect.width = info->imageX-currRect.x;
        currRect.height = info->imageY-currRect.y;
      }
    }
  }

  // }}}
  mouse;
  
  
  (*gui.getValue<DrawHandle>("image"))->add(&mouse);
}

void run(){
  static GenericGrabber g("dc,pwc,file","file=images/*.ppm.gz");
  g.setDesiredSize(imageSize);


    
  while(1){
    mutex.lock();
    g.grab()->convert(&currImage);
    mutex.unlock();
    
    static DrawHandle &image = gui.getValue<DrawHandle>("image");
    static DrawHandle &templ = gui.getValue<DrawHandle>("templ");
    static DrawHandle &buf = gui.getValue<DrawHandle>("buf");
    static float &significance = gui.getValue<float>("significance");
    static bool &useMasks = gui.getValue<bool>("use-masks");
    static bool &clipBuffers = gui.getValue<bool>("clip-buffers");
    static bool &mode = gui.getValue<bool>("mode");
    static FPSHandle &fps = gui.getValue<FPSHandle>("fps");


    fps.update();

    image = &currImage;
    
    (*image)->lock();
    (*image)->reset();
    if(dragging){
      (*image)->color(255,0,0,200);
      (*image)->fill(255,255,255,50);
      (*image)->rect(currRect);
    }else if(currTempl.getDim()){

      static ViewBasedTemplateMatcher matcher;
      matcher.setSignificance(significance);
      matcher.setMode(mode ? ViewBasedTemplateMatcher::crossCorrelation : ViewBasedTemplateMatcher::sqrtDistance);
      matcher.setClipBuffersToROI(clipBuffers);
      
      const std::vector<Rect> &rs = matcher.match(currImage,currTempl,useMasks?imageMask:Img8u::null,useMasks?templMask:Img8u::null);
      
      (*image)->color(255,0,0,200);
      (*image)->fill(255,255,255,0);
      for(unsigned int i=0;i<rs.size();++i){
        (*image)->rect(rs[i]);
      }

      buf = matcher.getBuffer();
      buf.update();

    }
    (*image)->unlock();
    image.update();

    

    
    mutex.lock();
    if(currTempl.getDim()){
      templ = &currTempl;
      templ.update();
    }
    mutex.unlock();

    
    Thread::msleep(10);
    
  }
}

int main(int n, char **ppc){
  QApplication app(n,ppc);
  
  init();
  exec_threaded(run);
  
  return app.exec();

}


#if 0
  
  ImgQ a = create("parrot");
  a = scale(a,0.5);
  a.setROI(Rect(110,70,50,20));
  
  ImgQ b = copyroi(a);
  a.setROI(Rect(20,20,300,400));
  

  Img8u a8 = cvt8u(a);
  Img8u b8 = cvt8u(b);
  
  Img8u ma = cvt8u(ones(a.getWidth(),a.getHeight(),1)*255);
  Img8u mb = cvt8u(ones(b.getWidth(),b.getHeight(),1)*255);
  
  ma.setROI(a.getROI());

  std::vector<Rect> rs = iclMatchTemplate(a8,&ma, b8, &mb, 0.95,0,0,0,false,0,false);
  
  for(unsigned int i=0;i<rs.size();++i){
    color(255,255,0);
    fill(0,0,0,0);
    rect(a,rs[i]);
  }

  show(a);
  show(scale(cvt(b8),5));
  

}
#endif //0
