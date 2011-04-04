#include <ICLQuick/Common.h>
#include <ICLFilter/ColorSegmentationOp.h>
#include <ICLFilter/MedianOp.h>
#include <ICLFilter/MorphologicalOp.h>
#include <ICLCC/CCFunctions.h>
#include <ICLCC/Color.h>
#include <ICLBlob/RegionDetector.h>

GUI gui("vsplit");
GenericGrabber grabber;
SmartPtr<ColorSegmentationOp> segmenter;
Mutex mutex;

Img8u currLUT,currLUTColor,segImage;

std::vector<ImageRegion> drawIM_AND_SEG;
std::vector<ImageRegion> drawLUT;
int hoveredClassID = -1;



void highlight_regions(int classID){
  hoveredClassID = classID;
  drawIM_AND_SEG.clear();
  drawLUT.clear();

    
  static RegionDetector rdLUT(1,1<<22,1,255);
  static RegionDetector rdSEG(1,1<<22,1,255);
  
  drawLUT = rdLUT.detect(&currLUT);

  if(classID < 1) return;
  const std::vector<ImageRegion> &rseg = rdSEG.detect(&segImage);

  for(unsigned int i=0;i<rseg.size();++i){
    if(rseg[i].getVal() == classID){
      drawIM_AND_SEG.push_back(rseg[i]);
    }
  }
}

void mouse(const MouseEvent &e){
  Mutex::Locker lock(mutex);
  if(!currLUT.getDim()) return;
  
  static const ICLWidget *wIM = *gui.getValue<DrawHandle>("image");
  static const ICLWidget *wLUT = *gui.getValue<DrawHandle>("lut");
  static const ICLWidget *wSEG = *gui.getValue<DrawHandle>("seg");

  Point p = e.getPos();
  
  if(e.isLeaveEvent()){
    highlight_regions(-1);
    return;
  }
  
  if(e.getWidget() == wLUT){
    highlight_regions(currLUT.getImageRect().contains(p.x,p.y) ?
                      currLUT(p.x,p.y,0) :
                      0 );
    if(e.isPressEvent()){
      gui["currClass"] = (hoveredClassID-1);
    }
  }else if (e.getWidget() == wIM){
    int cc = gui["currClass"]; 
    int r = gui["radius"];
    std::vector<double> c = e.getColor();
    if(c.size() == 3){
      if(e.isLeft()){
        segmenter->lutEntry(formatRGB,(int)c[0],(int)c[1],(int)c[2],r,r,r, (!gui["lb"].as<bool>()) * (cc+1) );
      }
        
      highlight_regions(segmenter->classifyPixel(c[0],c[1],c[2]));
      
      if(e.isRight()){
        gui["currClass"] = (hoveredClassID-1);
      }
    }
  }else if(e.getWidget() == wSEG){
    highlight_regions(segImage.getImageRect().contains(p.x,p.y) ?
                      segImage(p.x,p.y,0) :
                      0);
    if(e.isPressEvent()){
      gui["currClass"] = (hoveredClassID-1);
    }
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

void clear_lut(){
  Mutex::Locker lock(mutex);
  segmenter->clearLUT(0);
}

void init(){
  std::ostringstream classes;
  int n = pa("-n");
  for(int i=1;i<=n;++i){
    classes << "class " << i << ',';
  }
  
  if(pa("-r")){
    GenericGrabber::resetBus();
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
                << ( GUI("hbox") 
                     << "combo(" + classes.str() + ")[@handle=currClass@label=current class]"
                     << "togglebutton(current class,background)[@label=left button@handle=lb]"
                   )
                << "slider(0,255,4)[@out=radius@label=color radius]"
                << ( GUI("hbox") 
                     <<"button(load)[@handle=load]"
                     << "button(save)[@handle=save]"
                   )
                << ( GUI("hbox") 
                     << "checkbox(pre median,unchecked)[@out=preMedian]"
                     << "checkbox(post median,unchecked)[@out=postMedian]"
                   )
                << ( GUI("hbox") 
                     << "checkbox(post dilation,unchecked)[@out=postDilatation]"
                     << "checkbox(post erosion,unchecked)[@out=postErosion]"
                   )
                << ( GUI("hbox") 
                     << "label(?)[@handle=time@label=time for segm.]"
                     << "fps(10)[@handle=fps@label=system fps]"
                   )
                << ( GUI("hbox") << "camcfg()" << "button(clear)[@handle=clear]" )
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
  gui["clear"].registerCallback(clear_lut);
}

void run(){
  static const Point xys[3]={Point(1,2),Point(0,2),Point(0,1)};
  gui_DrawHandle(image);
  gui_DrawHandle(lut);
  gui_DrawHandle(seg);
  gui_LabelHandle(time);
  gui_bool(preMedian);
  gui_bool(postMedian);

  gui_bool(postErosion);
  gui_bool(postDilatation);

  int zAxis = gui["zAxis"];
  gui_int(z);
  const Img8u *grabbedImage = grabber.grab()->asImg<icl8u>();

  Mutex::Locker lock(mutex);
  
  if(preMedian){
    static MedianOp m(Size(3,3));
    grabbedImage = m.apply(grabbedImage)->asImg<icl8u>();
  }
  
  image = grabbedImage;
  Time t = Time::now();
  segImage = *segmenter->apply(grabbedImage)->asImg<icl8u>();

  if(postMedian){
    static MedianOp m(Size(3,3));
    segImage = *m.apply(&segImage)->asImg<icl8u>();
  }

  if(postErosion){
    static MorphologicalOp m(MorphologicalOp::erode3x3);
    segImage = *m.apply(&segImage)->asImg<icl8u>();
  }
  if(postDilatation){
    static MorphologicalOp m(MorphologicalOp::dilate3x3);
    segImage = *m.apply(&segImage)->asImg<icl8u>();
  }

  seg = &segImage;
  time = str(t.age().toMilliSecondsDouble())+"ms";

  

  currLUT = segmenter->getLUTPreview(xys[zAxis].x,xys[zAxis].y,z);
  currLUTColor = segmenter->getColoredLUTPreview(xys[zAxis].x,xys[zAxis].y,z);
 
  highlight_regions(hoveredClassID);
  
  //--lut--
  lut = &currLUTColor;
  lut->lock();
  lut->reset();
  lut->linewidth(2);
  
  for(unsigned int i=0;i<drawLUT.size();++i){
    float x = drawLUT[i].getCOG().x, y=drawLUT[i].getCOG().y;
    lut->color(255,255,255,255);
    lut->linestrip(drawLUT[i].getBoundary(false));
    lut->color(0,0,0,255);
    lut->text(str(drawLUT[i].getVal()),x+0.1, y+0.1, -2);
    lut->color(255,255,255,255);
    lut->text(str(drawLUT[i].getVal()),x, y, -2);
    if(drawLUT[i].getVal() == hoveredClassID){
      lut->color(0,100,255,255);
      lut->fill(0,100,255,40);
      lut->rect(drawLUT[i].getBoundingBox().enlarged(1));
    }
  }
  lut->unlock();
  lut.update();
  
  //--image and seg--
  ICLDrawWidget *ws[2] = {*image,*seg};
  for(int i=0;i<2;++i){
    ws[i]->lock();
    ws[i]->reset();
    ws[i]->linewidth(2);
    ws[i]->color(255,255-i*255,255-i*255,255);
    for(unsigned int j=0;j<drawIM_AND_SEG.size();++j){
      ws[i]->linestrip(drawIM_AND_SEG[j].getBoundary());
    }
    ws[i]->unlock();
    ws[i]->updateFromOtherThread();
  }
  
  gui["fps"].update();
}

int main(int n,char **args){
  return ICLApp(n,args,"-shifts|-s(int=8,int=0,int=0) "
                "-seg-format|-f(format=YUV) "
                "[m]-input|-i(2) "
                "-num-classes|-n(int=12) "
                "-load|-l(filename) "
                "-reset-bus|-r",
                init,run).exec();
}
