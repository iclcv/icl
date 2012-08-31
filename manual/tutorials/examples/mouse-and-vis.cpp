#include <ICLQt/Common.h>
#include <ICLCV/RegionDetector.h>
#include <ICLFilter/ColorDistanceOp.h>

GUI gui;
GenericGrabber grabber;

// connected components ([100-10^9] is the size range for
// detected regions, [255,255] is the value range
RegionDetector rd(100,1E9,255,255);

// creats a binarized color distance map with given
// reference color and initial threshold
ColorDistanceOp cd(Color(0,120,240),100);

// mouse callback: The given mouse event class provides
// information about the clicked pixel, i.e., it's color
// and it's image position (not used here)
void mouse(const MouseEvent &e){
  if(e.isLeft()) cd.setReferenceColor(e.getColor());
}

// initializiation function
void init(){
  grabber.init(pa("-i"));
  gui << Draw().handle("image")
      << Slider(0,200,30).out("t").maxSize(90,2)
      << Show();

  // install mouse handler
  gui["image"].install(mouse);
}

void run(){
  // extract current threshold from gui slider
  cd.setThreshold(gui["t"]);
  
  // extract handle
  DrawHandle draw = gui["image"];

  // grab image
  const ImgBase *I = grabber.grab();

  // set the background image
  draw = I;
 
  // set drawing properties
  draw->color(255,0,0);

  // extract regions in the binarized color distance map
  std::vector<ImageRegion> rs = rd.detect(cd.apply(I));

  // visualize every region
  for(size_t i=0;i<rs.size();++i){
    draw->linestrip(rs[i].getBoundary());
  }
  // actually re-render the component
  draw->render();
}
int main(int n,char **v){
  return ICLApp(n,v,"-input|-i(2)",init,run).exec();
}
