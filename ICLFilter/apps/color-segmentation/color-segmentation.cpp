/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/apps/color-segmentation/color-segmentation.c **
**          pp                                                     **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLQt/Common.h>
#include <ICLFilter/ColorSegmentationOp.h>
#include <ICLFilter/MedianOp.h>
#include <ICLFilter/MorphologicalOp.h>
#include <ICLCore/CCFunctions.h>
#include <ICLCore/Color.h>
#include <ICLCV/RegionDetector.h>

#include <ICLGeom/Scene.h>
#include <ICLGeom/GeomDefs.h>

VSplit gui;

#define MAX_LUT_3D_DIM 1000000

GenericGrabber grabber;
SmartPtr<ColorSegmentationOp> segmenter;
Mutex mutex;

Img8u currLUT,currLUTColor,segImage;

std::vector<ImageRegion> drawIM_AND_SEG;
std::vector<ImageRegion> drawLUT;
int hoveredClassID = -1;

void cc_util_hls_to_rgb_i(int h, int l, int s, int &r, int &g, int &b){
  float R,G,B;
  return cc_util_hls_to_rgb(h,l,s,R,G,B);
  r = R;
  g = G;
  b = B;
}
void rgb_id(int r, int g, int b, int &r2, int &g2, int &b2){
  r2=r; g2=g; b2=b;
}

Scene scene;

struct LUT3DSceneObject : public SceneObject {
  int w,h,t,dx,dy,dz,dim;
  std::vector<int> rs,gs,bs;
  Mutex mutex;
  virtual void lock(){ mutex.lock(); }
  virtual void unlock(){ mutex.unlock(); }
  
  LUT3DSceneObject(){

    segmenter->getLUTDims(w,h,t);
    dim = w*h*t;
    rs.resize(dim);
    gs.resize(dim);
    bs.resize(dim);
    format f = segmenter->getSegmentationFormat();
    void (*cc_func)(int,int,int,int&,int&,int&) = ( f == formatYUV ? cc_util_yuv_to_rgb :
                                                    f == formatHLS ? cc_util_hls_to_rgb_i :
                                                    rgb_id );
    float cx = float(w)/2;
    float cy = float(h)/2;
    float cz = float(t)/2;
    dx = 256/w;
    dy = 256/h;
    dz = 256/t;
    int i=0;
    for(int z=0;z<t;++z){
      for(int y=0;y<h;++y){
        for(int x=0;x<w;++x,++i){
          cc_func(x*dx,y*dy,z*dz,rs[i],gs[i],bs[i]);
          SceneObject *o = addCube(x-cx,y-cy,z-cz,1);
          o->setColor(Primitive::quad, GeomColor(rs[i],gs[i],bs[i],255));
          o->setColor(Primitive::line, GeomColor(255,255,255,255));
          o->setVisible(Primitive::line,false);
          o->setVisible(Primitive::vertex,false);
        }
      }
    }
    SceneObject *o = addCuboid(0,0,0,w,h,t);
    o->setVisible(Primitive::line,true);
    o->setVisible(Primitive::quad,false);
    o->setVisible(Primitive::vertex,false);
    o->setColor(Primitive::line,GeomColor(255,255,255,255));

    
    float wl = 1.3*(w/2);
    float hl = 1.3*(h/2);
    float tl = 1.3*(t/2);

    addVertex(Vec(-wl,0,0,1),geom_invisible());
    addVertex(Vec(wl,0,0,1),geom_invisible());
    addVertex(Vec(0,-hl,0,1),geom_invisible());
    addVertex(Vec(0,hl,0,1),geom_invisible());
    addVertex(Vec(0,0,-tl,1),geom_invisible());
    addVertex(Vec(0,0,tl,1),geom_invisible());
    
    addLine(0,1,geom_red(255));
    addLine(2,3,geom_green(255));
    addLine(4,5,geom_blue(255));
    
    
    for(int i=0;i<6;++i){
      std::string s = (i&1)? "" : "-";
      switch(segmenter->getSegmentationFormat()){
        case formatYUV: s += "yuv"[i/2]; break;
        case formatRGB: s += "rgb"[i/2]; break;
        case formatHLS: s += "hls"[i/2]; break;
        default:
          throw ICLException("invalid segmentation format");
      }
      addText(i,s,2);
    }
    
    
  }
  
  void update(float alpha){
    const icl8u *lut = segmenter->getLUT();
    for(int i=0;i<dim;++i){
      m_children[i]->setVisible( lut[i] );
      m_children[i]->setColor(Primitive::quad,GeomColor(rs[i],gs[i],bs[i],alpha));
      m_children[i]->setVisible(Primitive::line,hoveredClassID == lut[i]);
    }
    // createDisplayList(); // this does not help yet, since we update
    // the object every cycle even if nothing was changed ...
  }
  
} *lut3D=0;

void init_3D_LUT(){
  lut3D = new LUT3DSceneObject;
  scene.addObject(lut3D);
}

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
  
  static const ICLWidget *wIM = *gui.get<DrawHandle>("image");
  static const ICLWidget *wLUT = *gui.get<DrawHandle>("lut");
  static const ICLWidget *wSEG = *gui.get<DrawHandle>("seg");

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
  
  gui << ( HSplit()
           << Draw().handle("image").minSize(16,12).label("camera image")
           << Draw().handle("seg").minSize(16,12).label("segmentation result")
           )
      << ( HSplit()  
           << (Tab("2D,3D").handle("tab")
               << ( HBox() 
                    << Draw().handle("lut").minSize(16,12).label("lut")
                    << ( VBox().maxSize(3,100).minSize(4,1)
                         << Combo("x,y,z").handle("zAxis")
                         << Slider(0,255,0,true).out("z").label("vis. plane")
                         )
                    )
               << ( HBox() 
                    << Draw3D(Size::VGA).handle("lut3D")
                    << Slider(0,255,200,true).maxSize(2,100).out("alpha").label("alpha")
                  )
               )
           << ( VBox()
                << ( HBox() 
                     << Combo(classes.str()).handle("currClass").label("current class")
                     << Button("current class","background").label("left button").handle("lb")
                   )
                << Slider(0,255,4).out("radius").label("color radius")
                
                << (HBox().label("smooth LUT")
                    << Slider(0,27,10).out("smoothThresh").label("threshold")
                    << Button("do it").handle("smooth")
                    )
                << ( HBox() 
                     <<Button("load").handle("load")
                     << Button("save").handle("save")
                   )
                << ( HBox() 
                     << CheckBox("pre median").out("preMedian")
                     << CheckBox("post median").out("postMedian")
                   )
                << ( HBox() 
                     << CheckBox("post dilation").out("postDilatation")
                     << CheckBox("post erosion").out("postErosion")
                   )
                << ( HBox() 
                     << Label("?").handle("time").label("time for segm.")
                     << Fps(10).handle("fps").label("system fps")
                   )
                << ( HBox() 
                     << CamCfg("") 
                     << Button("clear").handle("clear") )
                )
           )
      << Show();


  gui["seg"].install(new MouseHandler(mouse));
  gui["image"].install(new MouseHandler(mouse));
  gui["lut"].install(new MouseHandler(mouse));
  
  DrawHandle lut = gui["lut"];
  DrawHandle seg = gui["seg"];

  lut->setRangeMode(ICLWidget::rmAuto);
  seg->setRangeMode(ICLWidget::rmAuto);

  gui["load"].registerCallback(load_dialog);
  gui["save"].registerCallback(save_dialog);
  gui["clear"].registerCallback(clear_lut);

  int dim =  ( (1+(0xff >> pa("-s",0).as<int>())) 
               *(1+(0xff >> pa("-s",1).as<int>())) 
               *(1+(0xff >> pa("-s",2).as<int>())) );
  if(dim <= MAX_LUT_3D_DIM){
    init_3D_LUT();
    scene.addCamera(Camera(Vec(0,0,100,1),Vec(0,0,-1,1),Vec(1,0,0,1)));
    DrawHandle3D lut3D = gui["lut3D"];
    lut3D->link(scene.getGLCallback(0));
    lut3D->install(scene.getMouseHandler(0));
  }
}

void run(){
  static ButtonHandle smooth = gui["smooth"];
  static int &smoothThresh = gui.get<int>("smoothThresh");

  if(smooth.wasTriggered()){
    icl8u *lut = segmenter->getLUT();
    int w, h, t;
    segmenter->getLUTDims(w,h,t);
    std::vector<icl8u> buf(w*h*t);

    std::vector<icl8u*> data;
    for(int i=0;i<t;++i){
      data.push_back(lut+w*h*i);
    }
    Img8u l(Size(w,h), t, data);
    int hs[256]={0}; 
    int n = pa("-n");
    for(int z=1;z<t-1;++z){
      for(int y=1;y<h-1;++y){
        for(int x=1;x<w-1;++x){
          std::fill(hs,hs+n+1,0);
          
          for(int zz=-1;zz<2;++zz){
            for(int yy=-1;yy<2;++yy){
              for(int xx=-1;xx<2;++xx){
                hs[ l(x+xx,y+yy,z+zz) ]++;
              }
            }
          }
          int imax = (int)(std::max_element(hs+1,hs+n+1) - hs);
          if(hs[imax] < smoothThresh) {
            buf[x + w*y + w*h * z] = 0;
          }else{
            buf[x + w*y + w*h * z] = imax;
          }
        }
      }
    }
    std::copy(buf.begin(),buf.end(),lut);


  }
  
  static const Point xys[3]={Point(1,2),Point(0,2),Point(0,1)};
  DrawHandle image = gui["image"];
  DrawHandle lut = gui["lut"];
  DrawHandle seg = gui["seg"];

  LabelHandle time = gui["time"];
  bool &preMedian = gui.get<bool>("preMedian");
  bool &postMedian = gui.get<bool>("postMedian");
  bool &postErosion = gui.get<bool>("postErosion");
  bool &postDilatation = gui.get<bool>("postDilatation");

  int zAxis = gui["zAxis"];
  int &z = gui.get<int>("z");
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
  lut.render();
  
  //--image and seg--
  ICLDrawWidget *ws[2] = {*image,*seg};
  for(int i=0;i<2;++i){
    ws[i]->linewidth(2);
    ws[i]->color(255,255-i*255,255-i*255,255);
    for(unsigned int j=0;j<drawIM_AND_SEG.size();++j){
      ws[i]->linestrip(drawIM_AND_SEG[j].getBoundary());
    }
    ws[i]->render();
  }
  
  gui["fps"].render();
  
  if(lut3D){
    lut3D->update(gui["alpha"]);
    gui["lut3D"].render();
  }
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
