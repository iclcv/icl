#include <ICLQuick/Common.h>
#include <ICLBlob/RegionDetector.h>
#include <ICLCC/Color.h>
#include <ICLQuick/QuickRegions.h>

// global data (GUI and reference color)
GUI gui("draw[@handle=draw@minsize=16x12]");
std::vector<double> refcol(3);
GenericGrabber grabber;
// reference color callback (ref. color is
// updated by mouse-click/drag)
void click_color(const MouseEvent &evt){
  if(evt.isLeft() && evt.getColor().size() == 3){
    refcol = evt.getColor();
  }
}

// initialization (create gui and install callback)
void init(){
  gui.show();
  gui["draw"].install(new MouseHandler(click_color));
  grabber.init(FROM_PROGARG("-input"));
  grabber.setIgnoreDesiredParams(true);
}


// working loop
void run(){
  
  Img32f im = cvt(grabber.grab());
  Img32f cm = colormap(im,refcol[0],refcol[1],refcol[2]);
  Img32f bi = thresh(cm,240);
  
  // create a region detector
  static RegionDetector rd(100,1<<20,255,255);
  const std::vector<icl::Region> &rs = rd.detect(&bi);

  gui_DrawHandle(draw);

  // visualization
  draw = im;
  draw->lock();
  draw->reset();
  draw->color(0,100,255);
  for(unsigned int i=0;i<rs.size();++i){
    // obtain region information (boundary pixels here)
    draw->linestrip(rs[i].getBoundary());
  }
  draw->unlock();
  draw.update();
}

// default main function
int main(int n, char **ppc){
  return ICLApp(n,ppc,"-input(2)",init,run).exec();
}
