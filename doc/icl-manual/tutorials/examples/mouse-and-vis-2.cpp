#include <ICLQt/Common.h>
#include <ICLCV/RegionDetector.h>
#include <ICLFilter/ColorDistanceOp.h>

GUI gui;
GenericGrabber grabber;
RegionDetector rd(100,1E9,255,255);
ColorDistanceOp cd(Color(0,120,240),100);

void mouse(const MouseEvent &e){
  if(e.isLeft()) cd.setReferenceColor(e.getColor());
}

void init(){
  grabber.init(pa("-i"));
  gui << Draw().handle("image")
      << Slider(0,200,30).handle("t").maxSize(90,2)
      << Show();

  gui["image"].install(mouse);
}

void run(){
  cd.setThreshold(gui["t"]);
  DrawHandle draw = gui["image"];
  const ImgBase *I = grabber.grab();

  draw = I;
  draw->color(255,0,0);

  std::vector<ImageRegion> rs = rd.detect(cd.apply(I));

  for(size_t i=0;i<rs.size();++i){
    draw->linestrip(rs[i].getBoundary());
  }

  draw->render();
}
int main(int n,char **v){
  return ICLApp(n,v,"-input|-i(2)",init,run).exec();
}
