#include <ICLQuick/Common.h>
#include <ICLFilter/ColorSegmentationOp.h>
#include <ICLCC/CCFunctions.h>
#include <ICLCC/Color.h>
#include <ICLBlob/RegionDetector.h>

GUI gui("hbox");
GenericGrabber grabber;
SmartPtr<ColorSegmentationOp> seg;

void mouse(const MouseEvent &e){
  int cc = gui["currClass"]; 
  int r = gui["radius"];
  if(e.isLeft() || e.isRight()){
    std::vector<double> c = e.getColor();
    if(c.size() == 3){
      seg->lutEntry(formatRGB,(int)c[0],(int)c[1],(int)c[2],r,r,r, e.isLeft() * (cc+1) );
    }
  }
}

void init(){
  std::ostringstream classes;
  int n = pa("-n");
  for(int i=1;i<=n;++i){
    classes << "class " << i << ',';
  }
  
  grabber.init(pa("-i"));
  grabber.useDesired(formatRGB);
  grabber.useDesired(depth8u);

  seg = new ColorSegmentationOp(pa("-s",0),pa("-s",1),pa("-s",2),pa("-f"));
  gui << "image[@handle=image@minsize=16x12@label=camera image]"
      << "draw[@handle=lut@minsize=16x12@label=lut]"
      << ( GUI("vbox") 
           << "combo(x,y,z)[@handle=zAxis]" 
           << "slider(0,255,0,vertical)[@out=z@label=vis. plane]"
         )
      << "draw[@handle=seg@minsize=16x12@label=segmentation result]"
      << ( GUI("vbox")
           << "combo(" + classes.str() + ")[@handle=currClass@label=current class]"
           << "slider(0,255,4)[@out=radius@label=color radius]"
           << "label(?)[@handle=time@label=time for segm.]"
         )
      << "!show";

  gui["image"].install(new MouseHandler(mouse));
  
  gui_DrawHandle(lut);
  gui_DrawHandle(seg);
  lut->setRangeMode(ICLWidget::rmAuto);
  seg->setRangeMode(ICLWidget::rmAuto);
}

void run(){
  const Img8u *image = grabber.grab()->asImg<icl8u>();
  
  gui["image"] = image;
  Time t = Time::now();
  gui["seg"] = seg->apply(image);
  gui["time"] = str(t.age().toMilliSecondsDouble())+"ms";

  gui["image"].update();
  gui["seg"].update();
  
  int a = gui["zAxis"];
  static const Point xys[3]={Point(1,2),Point(0,2),Point(0,1)};
  gui_DrawHandle(lut);
  lut = &seg->getColoredLUTPreview(xys[a].x,xys[a].y,gui["z"]);
  lut->lock();
  lut->reset();
  lut->linewidth(2);
  static RegionDetector rd(1,1<<22,1,255);
  const std::vector<ImageRegion> &rs = rd.detect(&seg->getLUTPreview(xys[a].x,xys[a].y,gui["z"]));
  for(unsigned int i=0;i<rs.size();++i){
    lut->color(255,255,255,255);
    lut->linestrip(rs[i].getBoundary(false));
    lut->color(255,0,0,255);
    lut->text(str(rs[i].getVal()),rs[i].getCOG().x, rs[i].getCOG().y, 9);
  }
  lut->unlock();
  lut.update();
}

int main(int n,char **args){
  /*
      Img32s a(Size(256,256),formatRGB);
      for(int u=0;u<256;++u){
        for(int v=0;v<256;++v){
        cc_util_yuv_to_rgb(128,u,v,a(u,v,0),a(u,v,1),a(u,v,2));
      }
      }
      show(a);

      a.print();
      return 0;
  */
  
  return ICLApp(n,args,"-shifts|-s(int=8,int=0,int=0) "
                "-seg-format|-f(format=YUV) "
                "[m]-input|-i(2) "
                "-num-classes|-n(int=12)",init,run).exec();
}
