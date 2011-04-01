#include <ICLQuick/Common.h>
#include <ICLFilter/ColorSegmentationOp.h>
#include <ICLCC/CCFunctions.h>
#include <ICLCC/Color.h>
#include <ICLBlob/RegionDetector.h>

GUI gui("vsplit");
GenericGrabber grabber;
SmartPtr<ColorSegmentationOp> segmenter;
Mutex mutex;

bool overImage = false;
bool overLUT = false;
bool overSeg = false;
void clear_over(){ overImage = overLUT = overSeg = false; }

Img8u currLUT,currLUTColor;
const std::vector<ImageRegion> *lutRegions=0;
const std::vector<ImageRegion> *segRegions=0;
std::vector<std::vector<Point> > rsA;
int hoveredClassID = -1;

void mouse(const MouseEvent &e){
  Mutex::Locker lock(mutex);
  if(!currLUT.getDim() || !segRegions) return;
  
  static const ICLWidget *wIM = *gui.getValue<DrawHandle>("image");
  static const ICLWidget *wLUT = *gui.getValue<DrawHandle>("lut");
  static const ICLWidget *wSEG = *gui.getValue<DrawHandle>("seg");

  Point p = e.getPos();
  
  if(e.isLeaveEvent()){
    rsA.clear();
    clear_over();
    hoveredClassID = -1;
    return;
  }
  
  if(e.getWidget() == wLUT){
    clear_over();
    overLUT=true;
    rsA.clear();
    if(currLUT.getImageRect().contains(p.x,p.y)){
      int classID = currLUT(p.x,p.y,0);
      if(classID > 0){
        for(unsigned int i=0;i<segRegions->size();++i){
          if((*segRegions)[i].getVal() == classID){
            rsA.push_back((*segRegions)[i].getBoundary(false));
          }
        }
        if(e.isPressEvent()){
          gui["currClass"] = (classID-1);
        }
      }
    }
  }else if (e.getWidget() == wIM){
    clear_over();
    overImage = true;
    
    int cc = gui["currClass"]; 
    int r = gui["radius"];
    std::vector<double> c = e.getColor();
    if(c.size() != 3) return;
    if(e.isLeft() || e.isRight()){
      segmenter->lutEntry(formatRGB,(int)c[0],(int)c[1],(int)c[2],r,r,r, e.isLeft() * (cc+1) );
    }else if(e.isMoveEvent()){
      hoveredClassID = segmenter->classifyPixel(c[0],c[1],c[2]);
      SHOW(hoveredClassID);
    }
  }else if(e.getWidget() == wSEG){
    clear_over();
    overSeg = true;
    
    
  }
}

void load_dialog(){
  try{
    segmenter->load(openFileDialog("PGM-Files (*.pgm);;Zipped PGM Files (*.pgm.gz);;All Files (*)"));
  }catch(...){}
}

void save_dialog(){
  try{
    segmenter->save(saveFileDialog("PGM-Files (*.pgm);;Zipped PGM Files (*.pgm.gz)"));
  }catch(...){}
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

  if( pa("-l").as<bool>() && pa("-s").as<bool>()){
    WARNING_LOG("program arguments -l and -s are exclusive:(-l is used here)");
  }
  
  segmenter = new ColorSegmentationOp(pa("-s",0),pa("-s",1),pa("-s",2),pa("-f"));
  
  
  if(pa("-l")){
    segmenter->load(pa("-l"));
  }
  
  
  gui << ( GUI("hsplit")
           << "draw[@handle=image@minsize=16x12@label=camera image]"
           << "draw[@handle=seg@minsize=16x12@label=segmentation result]"
         )
      << ( GUI("hsplit")  
           << ( GUI("hbox") 
                << "draw[@handle=lut@minsize=16x12@label=lut]"
                << ( GUI("vbox[@maxsize=3x100@minsize=4x1]") 
                     << "combo(x,y,z)[@handle=zAxis]" 
                     << "slider(0,255,0,vertical)[@out=z@label=vis. plane]"
                   )
              )
           << ( GUI("vbox")
                << "combo(" + classes.str() + ")[@handle=currClass@label=current class]"
                << "slider(0,255,4)[@out=radius@label=color radius]"
                << "button(load)[@handle=load]"
                << "button(save)[@handle=save]"
                << "label(?)[@handle=time@label=time for segm.]"
                )
           )
      << "!show";

  gui["seg"].install(new MouseHandler(mouse));
  gui["image"].install(new MouseHandler(mouse));
  gui["lut"].install(new MouseHandler(mouse));
  
  gui_DrawHandle(lut);
  gui_DrawHandle(seg);
  lut->setRangeMode(ICLWidget::rmAuto);
  seg->setRangeMode(ICLWidget::rmAuto);

  gui["load"].registerCallback(load_dialog);
  gui["save"].registerCallback(save_dialog);
}

void run(){
  static const Point xys[3]={Point(1,2),Point(0,2),Point(0,1)};
  gui_DrawHandle(image);
  gui_DrawHandle(lut);
  gui_DrawHandle(seg);
  gui_LabelHandle(time);
  int zAxis = gui["zAxis"];
  gui_int(z);
  const Img8u *grabbedImage = grabber.grab()->asImg<icl8u>();
  
  image = grabbedImage;
  Time t = Time::now();
  const ImgBase *segImage = segmenter->apply(grabbedImage);
  seg = segImage;
  time = str(t.age().toMilliSecondsDouble())+"ms";

  
  Mutex::Locker lock(mutex);
  currLUT = segmenter->getLUTPreview(xys[zAxis].x,xys[zAxis].y,z);
  currLUTColor = segmenter->getColoredLUTPreview(xys[zAxis].x,xys[zAxis].y,z);
  
  //--lut--
  lut = &currLUTColor;
  lut->lock();
  lut->reset();
  lut->linewidth(2);
  static RegionDetector rdLUT(1,1<<22,1,255);
  static RegionDetector rdSEG(1,1<<22,1,255);

  lutRegions = &rdLUT.detect(&currLUT);
  segRegions = &rdSEG.detect(segImage);
  
  const std::vector<ImageRegion> &rs = *lutRegions;
  for(unsigned int i=0;i<rs.size();++i){
    lut->color(255,255,255,255);
    lut->linestrip(rs[i].getBoundary(false));
    lut->color(0,0,0,255);
    lut->text(str(rs[i].getVal()),rs[i].getCOG().x+0.1, rs[i].getCOG().y+0.1, -2);
    lut->color(255,255,255,255);
    lut->text(str(rs[i].getVal()),rs[i].getCOG().x, rs[i].getCOG().y, -2);
    if(rs[i].getVal() == hoveredClassID){
      lut->color(0,100,255,255);
      lut->fill(0,100,255,40);
      lut->rect(rs[i].getBoundingBox().enlarged(1));
    }
  }
    
  lut->unlock();

  //--image--
  image->lock();
  image->reset();
  image->linewidth(2);
  image->color(255,255,255,255);
  for(unsigned int i=0;i<rsA.size();++i){
    image->linestrip(rsA[i]);
  }
  image->unlock();


  //--seg--
  seg->lock();
  seg->reset();
  seg->linewidth(2);
  seg->color(255,0,0,255);
  for(unsigned int i=0;i<rsA.size();++i){
    seg->linestrip(rsA[i]);
  }
  seg->unlock();
  

  //--update-- all
  lut.update();
  image.update();
  seg.update();

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
                "-num-classes|-n(int=12) "
                "-load|-l(filename) ",
                init,run).exec();
}
