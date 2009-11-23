#include <iclCommon.h>
#include <iclRegionDetector.h>
#include <iclColor.h>

// global data (GUI and reference color)
GUI gui("draw[@handle=draw@minsize=16x12]");
Color refcol;

// reference color callback (ref. color is
// updated by mouse-click/drag)
void click_color(const MouseEvent &evt){
  if(evt.isLeft() && evt.getColor().size() == 3){
    std::copy(evt.getColor().begin(),
              evt.getColor().end(),
              refcol.begin());
  }
}

// initialization (create gui and install callback)
void init(){
  gui.show();
  gui["draw"].install(new MouseHandler(click_color));
}

// color distance computation functor
struct DistMap{
  Color refcol;
  DistMap(const Color &color):refcol(color){}
  void operator()(const icl8u src[3],icl8u dst[1]){
    int err = abs(src[0]-refcol[0])
             +abs(src[1]-refcol[1])
             +abs(src[2]-refcol[2]);
    *dst = 255*(err<50);
  }
};

// working loop
void run(){
  gui_DrawHandle(draw);
  static GenericGrabber grabber(FROM_PROGARG("-input"));
  
  // akquire new image
  static Img8u image;
  grabber.grab()->convert(&image);
  
  // apply color distance functor
  static Img8u bin(image.getSize(),1);
  image.reduce_channels<icl8u,3,1,DistMap>(bin,DistMap(refcol));
  
  // create a region detector
  static RegionDetector rd(100,1<<20,255,255);
  const std::vector<icl::Region> &regions = rd.detect(&bin);
  

  // visualization
  draw = image;
  draw->lock();
  draw->reset();
  draw->color(0,100,255);
  for(unsigned int i=0;i<regions.size();++i){
    // obtain region information (boundary pixels here)
    draw->linestrip(regions[i].getBoundary());
  }
  draw->color(255,0,0);
  draw->text("click on reference color",5,225,-1,-1,8);
  draw->unlock();
  draw.update();
}

// default main function
int main(int n, char **ppc){
  return ICLApp(n,ppc,"-input(2)",init,run).exec();
}
