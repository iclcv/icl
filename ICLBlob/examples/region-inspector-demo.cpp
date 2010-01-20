#include <ICLQuick/Common.h>
#include <ICLBlob/RegionDetector.h>

GUI gui("hsplit");
RegionDetector rd;
Mutex mutex;

struct MouseIO : public MouseHandler{
  int x,y;
  MouseIO():x(0),y(0){}
  virtual void process(const MouseEvent &evt){
    x = evt.getX();
    y = evt.getY();
  }
} mouseIO;


void init(){
  gui << "draw[@minsize=32x24@label=image@handle=image]";
  
  GUI labels("vbox[@label=Region information]");
  labels << "label()[@label=Region Size@handle=size-handle@minsize=6x2]";
  labels << "label()[@label=Region COG@handle=cog-handle@minsize=6x2]";
  labels << "label()[@label=Region Value@handle=val-handle@minsize=6x2]";
  labels << "label()[@label=Region Form Factor@handle=ff-handle@minsize=6x2]";
  labels << "label()[@label=Region EV-Ratio@handle=evratio-handle@minsize=6x2]";
  labels << "label()[@label=Region Boundary Length@handle=bl-handle@minsize=6x2]";
  labels << ( GUI("hbox") 
              << "togglebutton(stopped,grabbing)[@out=grabbing@handle=grab-handle@minsize=3x2]"
              << "button(grab next)[@handle=grab-next-handle@minsize=3x2]"
              );
  labels << "slider(2,256,256)[@out=levels@label=reduce levels]";
  
  
  gui << labels;
  
  gui.show();
}


struct RegionContainsPoint{
  inline RegionContainsPoint(const Point &p):p(p){}
  Point p;
  inline bool operator()(const ScanLine &sls) const{
    return p.y == sls.y && p.x >= sls.x && p.x < sls.x+sls.len;
  }
  inline bool operator()(const icl::Region &rs) const{
    const vector<ScanLine> &sls = rs.getScanLines();
    return std::find_if(sls.begin(),sls.end(),*this) != sls.end();
  }
};

void run(){
  string file;
  for(unsigned int i=0;i<pa_argcount();++i){
    file += pa_arg<string>(i) + " ";
  }
  static GenericGrabber g(FROM_PROGARG("-input"));
  g.setDesiredSize(parse<Size>(pa_subarg<string>("-size",0,"640x480")));
  g.setIgnoreDesiredParams(false);
  g.setDesiredFormat(formatGray);

  static ICLDrawWidget &d = **gui.getValue<DrawHandle>("image");
  d.install(&mouseIO);

  static LabelHandle &valHandle = gui.getValue<LabelHandle>("val-handle");
  static LabelHandle &cogHandle = gui.getValue<LabelHandle>("cog-handle");
  static LabelHandle &sizeHandle = gui.getValue<LabelHandle>("size-handle");
  static LabelHandle &ffHandle = gui.getValue<LabelHandle>("ff-handle");
  static LabelHandle &evratioHandle = gui.getValue<LabelHandle>("evratio-handle");
  static LabelHandle &blHandle = gui.getValue<LabelHandle>("bl-handle");
  static bool &grabButtonDown = gui.getValue<bool>("grabbing");
  static ButtonHandle &grabNextHandle = gui.getValue<ButtonHandle>("grab-next-handle");
  int &levels = gui.getValue<int>("levels");

  static int lastLevels = levels;
  static const Img8u *grabbedImage;
  static Img8u reducedLevels;  

  const Img8u *useImage = 0;
  const std::vector<icl::Region> *rs = 0;
  while(1){
    if(grabNextHandle.wasTriggered() || !useImage || grabButtonDown){
      grabbedImage = g.grab()->asImg<icl8u>();
      useImage = grabbedImage;

      if(levels != 256){
        reducedLevels = cvt8u(icl::levels(cvt(grabbedImage),levels));
        useImage = &reducedLevels;
      }
      
      d.setImage(useImage);
      rs = &rd.detect(useImage);
    }else if(lastLevels != levels){
      if(levels != 256){
        reducedLevels = cvt8u(icl::levels(cvt(grabbedImage),levels));
        useImage = &reducedLevels;
      }else{
        useImage = grabbedImage;
      }
    }
    lastLevels = levels;
    
    d.lock();
    d.reset();
    
    mutex.lock();
    Point m(mouseIO.x,mouseIO.y);
    mutex.unlock();
  
    if(useImage->getImageRect().contains(m.x,m.y)){
      // find the region, that contains mouseX,mouseY
      std::vector<icl::Region>::const_iterator it = find_if(rs->begin(),rs->end(),RegionContainsPoint(m));
      if(it != rs->end()){
        icl::Region r = *it;
        
        const std::vector<Point> &boundary = r.getBoundary();
        const Rect &bb = r.getBoundingBox();
        
        d.color(255,0,0,255);
        d.nofill();
        d.rect(bb);
        d.color(0,150,255,200);
        d.linestrip(boundary);
        
        valHandle = r.getVal();
        cogHandle = str(r.getCOG());
        sizeHandle = r.getSize();
        ffHandle = r.getFormFactor();
        evratioHandle = r.getPCAInfo().len2/r.getPCAInfo().len1;
        blHandle = r.getBoundaryLength();
      }else{
        valHandle = "no region";
        cogHandle = "";
        sizeHandle = "";
        ffHandle = "";
        evratioHandle = "";
        blHandle = "";
      
      }
    }
    
    d.unlock();
    d.updateFromOtherThread();
    
    Thread::msleep(10);
  }
}


int main(int n, char **ppc){
  ExecThread x(run);
  QApplication app(n,ppc);
  
  pa_explain("-input","define device parameters (e.g. -d dc 0 or -d file image/*.ppm)");
  pa_explain("-size","define image size (e.g. -size 640x480)");
  pa_init(n,ppc,"-input(2) -size(1)");

  init();
  
  x.run();
  
  return app.exec();
}
