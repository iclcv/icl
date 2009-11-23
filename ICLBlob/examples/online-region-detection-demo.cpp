#include <iclCommon.h>
#include <iclRegionDetector.h>
#include <iclColor.h>

GUI gui("draw[@handle=draw@minsize=16x12]");
Color refcol;

void click_color(const MouseEvent &evt){
  if(evt.isLeft() && evt.getColor().size() == 3){
    std::copy(evt.getColor().begin(),
              evt.getColor().end(),
              refcol.begin());
    std::cout << "new reference color:" << refcol.transp() << std::endl;
  }
}

void init(){
  gui.show();
  gui["draw"].install(new MouseHandler(click_color));
}

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

void run(){
  gui_DrawHandle(draw);
  static GenericGrabber grabber(FROM_PROGARG("-input"));
  
  static Img8u image;
  grabber.grab()->convert(&image);
  static Img8u bin(image.getSize(),1);
  image.reduce_channels<icl8u,3,1,DistMap>(bin,DistMap(refcol));
  
  static RegionDetector rd(100,1<<20,255,255);
  const std::vector<icl::Region> &regions = rd.detect(&bin);
  

  draw = image;
  draw->lock();
  draw->reset();
  draw->color(0,100,255);
  for(unsigned int i=0;i<regions.size();++i){
    draw->linestrip(regions[i].getBoundary());
  }
  draw->color(255,0,0);
  draw->text("click on reference color",5,225,-1,-1,8);
  draw->unlock();
  draw.update();
}


int main(int n, char **ppc){
  return ICLApp(n,ppc,"-input(2)",init,run).exec();
}
