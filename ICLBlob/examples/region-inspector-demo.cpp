#include <iclQuick.h>
#include <iclQt.h>
#include <iclProgArg.h>
#include <iclRegionDetector.h>
#include <iclGenericGrabber.h>
#include <iclStringUtils.h>

#include <algorithm>

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
  static GenericGrabber g(pa_subarg<std::string>("-d",0,"pwc"),
                          pa_subarg<std::string>("-d",0,"pwc")+"="+
                          pa_subarg<std::string>("-d",1,"0"));
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

  const Img8u *image = 0;
  const std::vector<icl::Region> *rs = 0;
  while(1){
    if(grabNextHandle.wasTriggered() || !image || grabButtonDown){
      image = g.grab()->asImg<icl8u>();
      d.setImage(image);
      rs = &rd.detect(image);
    }
    d.lock();
    d.reset();
    
    mutex.lock();
    Point m(mouseIO.x,mouseIO.y);
    mutex.unlock();
  
    if(image->getImageRect().contains(m.x,m.y)){
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
  
  pa_explain("-d","define device parameters (e.g. -d dc 0 or -d file image/*.ppm)");
  pa_explain("-size","define image size (e.g. -size 640x480)");
  pa_init(n,ppc,"-d(2) -size(1)");

  init();
  
  x.run();
  
  return app.exec();
}
