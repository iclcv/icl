#include <ICLQt/Qt.h>
#include <ICLAlgorithms/UsefulFunctions.h>
#include <ICLQuick/Quick.h>
#include <ICLIO/GenericGrabber.h>
#include <ICLBlob/RegionDetector.h>
#include <ICLAlgorithms/ViewBasedTemplateMatcher.h>

Size imageSize(640,480);
GUI gui("hbox");
Img8u currTempl;
Img8u templMask(Size(1,1),1);

Img8u currImage;
Img8u imageMask = cvt8u(ones(imageSize.width,imageSize.height,1)*255);

Mutex mutex;
bool dragging = false;
Rect currRect = Rect(Point::null,imageSize).enlarged(-imageSize.width/2);

bool dragging_R  = false;
Rect currROI(Point::null,imageSize);


void mouse(const MouseEvent &e){
  if(e.isPressEvent()){
    if(e.isLeft()){
      currRect = Rect(e.getPos(),Size(1,1));
      dragging_R=false;
    }else{
      currROI = Rect(e.getPos(),Size(1,1));
      dragging_R=true;
    }
      dragging = true;
  }else if(e.isReleaseEvent()){
    Mutex::Locker l(mutex);
    if(dragging_R){
      currROI = currROI.normalized() & Rect(Point::null,imageSize);
      dragging = false;
    }else{
      Rect r = currRect.normalized() & Rect(Point::null,imageSize);
      currImage.setROI(r);
      currTempl.setSize(r.getSize());
      currImage.deepCopyROI(&currTempl);
      templMask.setSize(currTempl.getSize());
      templMask.clear(-1,255,false);
      currImage.setFullROI();
      dragging =false;
    }
  }else if(e.isDragEvent()){
    if(dragging_R){
      currROI.width = e.getX()-currROI.x;
      currROI.height = e.getY()-currROI.y;
    }else{
      currRect.width = e.getX()-currRect.x;
      currRect.height = e.getY()-currRect.y;
    }
  }
}

void init(){
  gui << "draw()[@label=image@minsize=32x24@handle=image]";
  gui << ( GUI("vbox") 
           << "draw()[@label=template@minsize=10x6@handle=templ]"
           << "draw()[@label=buffer@minsize=10x6@handle=buf]"
          );
           

  GUI controls("vbox[@minsize=7x7]");
  controls << "fslider(0,1,0.9)[@handle=significance-handle@label=significance@out=significance]";
  controls << "fps(50)[@handle=fps@minsize=5x5]";
  controls << "togglebutton(no masks, with masks)[@out=use-masks]";
  controls << "togglebutton(dont clip buffers,clip buffers)[@out=clip-buffers]";
  controls << "togglebutton(square distance,norm. cross corr)[@out=mode]";
  
  gui << controls;
   
  gui.show();

 
  
  (*gui.getValue<DrawHandle>("image"))->install(new MouseHandler(mouse));
}

void vis_roi(ICLDrawWidget *w){
  Rect r = currROI.normalized();
  w->color(0,0,0,0);
  w->fill(0,0,0,200);
  w->rect(Rect(Point::null,Size(imageSize.width,r.y)));
  w->rect(Rect(0,r.y,r.x,imageSize.height-r.y));
  w->rect(Rect(r.right(),r.y,imageSize.width-r.right(),imageSize.height-r.y));
  w->rect(Rect(r.x,r.bottom(),r.width,imageSize.height-r.bottom()));
  
  w->color(200,0,255,200);
  w->fill(255,255,255,0);
  w->rect(r);
}


void run(){
  static GenericGrabber g(FROM_PROGARG("-input"));
  g.setDesiredSize(imageSize);


    
  while(1){
    mutex.lock();
    g.grab()->convert(&currImage);
    if(!dragging){
      currImage.setROI(currROI.normalized() & currImage.getImageRect());
    }
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
      if(dragging_R){
        vis_roi(*image);
      }else{
        (*image)->color(255,0,0,200);
        (*image)->fill(255,255,255,50);
        (*image)->rect(currRect);
      }      
    }else if(currTempl.getDim()){
      
      if(currROI != currImage.getImageRect()){
        vis_roi(*image);
      }
      
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
  pa_explain("-input","define input device (like -input dc 0 or -input file images/image.ppm)");
  pa_init(n,ppc,"-input(2)");
  ExecThread x(run);
  QApplication app(n,ppc);
  
  init();
  x.run();
  
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
