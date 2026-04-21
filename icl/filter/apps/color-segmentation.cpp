// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/Common2.h>
#include <icl/filter/ColorSegmentationOp.h>
#include <icl/filter/MedianOp.h>
#include <icl/filter/MorphologicalOp.h>
#include <icl/core/CCFunctions.h>
#include <icl/core/Color.h>
#include <icl/cv/RegionDetector.h>

#include <icl/geom2/Scene2.h>
#include <icl/geom2/GroupNode.h>
#include <icl/geom2/CuboidNode.h>
#include <icl/geom2/MeshNode.h>
#include <icl/geom2/TextNode.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <icl/geom/Camera.h>
#include <icl/geom/GeomDefs.h>
#include <icl/geom/Material.h>
#include <mutex>

using namespace icl::geom2;

VSplit gui;

constexpr int MAX_LUT_3D_DIM = 10000000;

GenericGrabber grabber;
std::shared_ptr<ColorSegmentationOp> segmenter;
std::recursive_mutex mtex;

Img8u currLUT,currLUTColor,segImage;

std::vector<ImageRegion> drawIM_AND_SEG;
std::vector<ImageRegion> drawLUT;
int hoveredClassID = -1;

void cc_util_hls_to_rgb_i(int h, int l, int s, int &r, int &g, int &b){
  float R,G,B;
  cc_util_hls_to_rgb(h,l,s,R,G,B);
  r = R;
  g = G;
  b = B;
}
void rgb_id(int r, int g, int b, int &r2, int &g2, int &b2){
  r2=r; g2=g; b2=b;
}

Scene2 scene;

// 3D LUT visualization state
struct LUT3D {
  std::shared_ptr<GroupNode> group;
  std::vector<std::shared_ptr<CuboidNode>> cubes;
  std::vector<int> rs, gs, bs;
  int dim = 0;

  void update(float alpha) {
    scene.lock();
    const icl8u *lut = segmenter->getLUT();
    for(int i = 0; i < dim; ++i){
      cubes[i]->setVisible(lut[i] != 0);
      cubes[i]->setMaterial(Material::fromColor(GeomColor(rs[i],gs[i],bs[i],alpha)));
      cubes[i]->setPrimitiveVisible(PrimLine, hoveredClassID == lut[i]);
    }
    scene.unlock();
  }
};
std::unique_ptr<LUT3D> lut3D;

void prepare_scene(){
  int dim = ( (1+(0xff >> pa("-s",0).as<int>()))
              *(1+(0xff >> pa("-s",1).as<int>()))
              *(1+(0xff >> pa("-s",2).as<int>())) );
  if(dim > MAX_LUT_3D_DIM) return;

  lut3D = std::make_unique<LUT3D>();
  lut3D->group = std::make_shared<GroupNode>();

  int w, h, t;
  segmenter->getLUTDims(w, h, t);
  lut3D->dim = w * h * t;
  lut3D->rs.resize(lut3D->dim);
  lut3D->gs.resize(lut3D->dim);
  lut3D->bs.resize(lut3D->dim);

  format f = segmenter->getSegmentationFormat();
  void (*cc_func)(int,int,int,int&,int&,int&) = ( f == formatYUV ? cc_util_yuv_to_rgb :
                                                   f == formatHLS ? cc_util_hls_to_rgb_i :
                                                   rgb_id );
  float cx = float(w) / 2;
  float cy = float(h) / 2;
  float cz = float(t) / 2;
  int dx = 256 / w;
  int dy = 256 / h;
  int dz = 256 / t;

  int i = 0;
  for(int z = 0; z < t; ++z){
    for(int y = 0; y < h; ++y){
      for(int x = 0; x < w; ++x, ++i){
        cc_func(w == 1 ? 127 : x * dx,
                h == 1 ? 127 : y * dy,
                t == 1 ? 127 : z * dz,
                lut3D->rs[i], lut3D->gs[i], lut3D->bs[i]);
        auto cube = CuboidNode::createCube(x - cx + 0.5f, y - cy + 0.5f, z - cz + 0.5f, 1);
        cube->setMaterial(Material::fromColors(
            GeomColor(lut3D->rs[i], lut3D->gs[i], lut3D->bs[i], 255),
            GeomColor(255, 255, 255, 255)));
        cube->setPrimitiveVisible(PrimLine | PrimVertex, false);
        lut3D->cubes.push_back(cube);
        lut3D->group->addChild(cube);
      }
    }
  }
  scene.addNode(lut3D->group);

  // Bounding box wireframe
  auto bbox = CuboidNode::create(0, 0, 0, w, h, t);
  bbox->setPrimitiveVisible(PrimLine, true);
  bbox->setPrimitiveVisible(PrimQuad | PrimVertex, false);
  bbox->setMaterial(Material::fromColor(GeomColor(255, 255, 255, 255)));
  scene.addNode(bbox);

  // Axis lines
  float wl = 1.3f * (w / 2.0f);
  float hl = 1.3f * (h / 2.0f);
  float tl = 1.3f * (t / 2.0f);

  auto axes = std::make_shared<MeshNode>();
  axes->addVertex(Vec(-wl, 0, 0, 1));
  axes->addVertex(Vec( wl, 0, 0, 1));
  axes->addVertex(Vec(0, -hl, 0, 1));
  axes->addVertex(Vec(0,  hl, 0, 1));
  axes->addVertex(Vec(0, 0, -tl, 1));
  axes->addVertex(Vec(0, 0,  tl, 1));
  axes->addLine(0, 1, GeomColor(255, 0, 0, 255));
  axes->addLine(2, 3, GeomColor(0, 255, 0, 255));
  axes->addLine(4, 5, GeomColor(0, 0, 255, 255));
  axes->setPrimitiveVisible(PrimQuad, false);
  axes->setPrimitiveVisible(PrimTriangle, false);
  scene.addNode(axes);

  // Axis labels as billboard TextNodes
  Vec labelPositions[6] = {
    Vec(-wl, 0, 0, 1), Vec(wl, 0, 0, 1),
    Vec(0, -hl, 0, 1), Vec(0, hl, 0, 1),
    Vec(0, 0, -tl, 1), Vec(0, 0, tl, 1)
  };
  for(int i = 0; i < 6; ++i){
    std::string s = (i & 1) ? "" : "-";
    switch(f){
      case formatYUV: s += "yuv"[i/2]; break;
      case formatRGB: s += "rgb"[i/2]; break;
      case formatHLS: s += "hls"[i/2]; break;
      default: throw ICLException("invalid segmentation format");
    }
    auto label = TextNode::create(s, 2, GeomColor(255, 255, 255, 255));
    label->translate(labelPositions[i][0], labelPositions[i][1], labelPositions[i][2]);
    scene.addNode(label);
  }

  // Camera + link to GUI
  scene.addCamera(Camera(Vec(0,0,100,1),Vec(0,0,-1,1),Vec(1,0,0,1)));
  scene.setBounds(50);
  gui["lut3D"].link(scene.getGLCallback(0).get());
  gui["lut3D"].install(scene.getMouseHandler(0));
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

  for(size_t i = 0; i < rseg.size(); ++i){
    if(rseg[i].getVal() == classID){
      drawIM_AND_SEG.push_back(rseg[i]);
    }
  }
}

void mouse_image(const MouseEvent &e){
  std::scoped_lock<std::recursive_mutex> lock(mtex);
  if(!currLUT.getDim()) return;
  if(e.isLeaveEvent()){ highlight_regions(-1); return; }

  int cc = gui["currClass"];
  int r = gui["radius"];
  std::vector<double> c = e.getColor();

  if(c.size() == 3){
    if(e.isLeft()){
      segmenter->lutEntry(formatRGB,(int)c[0],(int)c[1],(int)c[2],r,r,r, (!gui["lb"].as<bool>()) * (cc+1));
    }

    highlight_regions(segmenter->classifyPixel(c[0],c[1],c[2]));

    if(e.isRight()){
      gui["currClass"] = (hoveredClassID-1);
    }
  }
}

void mouse_lut(const MouseEvent &e){
  std::scoped_lock<std::recursive_mutex> lock(mtex);
  if(!currLUT.getDim()) return;

  if(e.isLeaveEvent()){
    highlight_regions(-1);
    return;
  }

  Point p = e.getPos();
  highlight_regions(currLUT.getImageRect().contains(p.x,p.y) ?
                    currLUT(p.x,p.y,0) : 0);
  if(e.isPressEvent()){
    gui["currClass"] = (hoveredClassID-1);
  }
}

void mouse_seg(const MouseEvent &e){
  std::scoped_lock<std::recursive_mutex> lock(mtex);
  if(!currLUT.getDim()) return;

  if(e.isLeaveEvent()){
    highlight_regions(-1);
    return;
  }

  Point p = e.getPos();
  highlight_regions(segImage.getImageRect().contains(p.x,p.y) ?
                    segImage(p.x,p.y,0) : 0);
  if(e.isPressEvent()){
    gui["currClass"] = (hoveredClassID-1);
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
  std::scoped_lock<std::recursive_mutex> lock(mtex);
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

  segmenter = std::make_shared<ColorSegmentationOp>(pa("-s",0),pa("-s",1),pa("-s",2),pa("-f"));

  if(pa("-l")){
    segmenter->load(pa("-l"));
  }

  gui << ( HSplit()
           << Canvas().handle("image").minSize(16,12).label("camera image")
           << Canvas().handle("seg").minSize(16,12).label("segmentation result")
           )
      << ( HSplit()
           << (Tab("2D,3D").handle("tab")
               << ( HBox()
                    << Canvas().handle("lut").minSize(16,12).label("lut")
                    << ( VBox().maxSize(3,100).minSize(4,1)
                         << Combo("x,y,z").handle("zAxis")
                         << Slider(0,255,0,true).out("z").label("vis. plane")
                         )
                    )
               << ( HBox()
                    << Canvas3D(Size::VGA).handle("lut3D")
                    << Slider(0,255,200,true).maxSize(2,100).out("alpha").label("alpha")
                  )
               )
           << ( VBox()
                << CheckBox("Pause Image Acquisition",false).handle("paused")
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


  gui["image"].install(mouse_image);
  gui["lut"].install(mouse_lut);
  gui["seg"].install(mouse_seg);

  DrawHandle lut = gui["lut"];
  DrawHandle seg = gui["seg"];

  lut->setRangeMode(ICLWidget::rmAuto);
  seg->setRangeMode(ICLWidget::rmAuto);

  gui["load"].registerCallback(load_dialog);
  gui["save"].registerCallback(save_dialog);
  gui["clear"].registerCallback(clear_lut);

  prepare_scene();
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

  static Image inputImage;
  if(!inputImage || !gui["paused"].as<bool>()){
    inputImage = grabber.grabImage();
  }else{
    Thread::msleep(50);
  }

  Image filtered = inputImage;

  std::scoped_lock<std::recursive_mutex> lock(mtex);

  if(preMedian){
    static MedianOp m(Size(3,3));
    m.apply(filtered, filtered);
  }

  image = filtered;
  Time t = Time::now();
  {
    Image segResult;
    segmenter->apply(filtered, segResult);
    segImage = segResult.as8u();
  }

  if(postMedian){
    static MedianOp m(Size(3,3));
    Image tmp;
    m.apply(Image(segImage), tmp);
    segImage = tmp.as8u();
  }

  if(postErosion){
    static MorphologicalOp m(MorphologicalOp::erode3x3);
    Image tmp;
    m.apply(Image(segImage), tmp);
    segImage = tmp.as8u();
  }
  if(postDilatation){
    static MorphologicalOp m(MorphologicalOp::dilate3x3);
    Image tmp;
    m.apply(Image(segImage), tmp);
    segImage = tmp.as8u();
  }

  seg = &segImage;
  time = str(t.age().toMilliSecondsDouble())+"ms";

  currLUT = segmenter->getLUTPreview(xys[zAxis].x,xys[zAxis].y,z);
  currLUTColor = segmenter->getColoredLUTPreview(xys[zAxis].x,xys[zAxis].y,z);

  highlight_regions(hoveredClassID);

  //--lut--
  lut = &currLUTColor;
  lut->linewidth(2);

  for(size_t i = 0; i < drawLUT.size(); ++i){
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
    for(size_t j = 0; j < drawIM_AND_SEG.size(); ++j){
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
  return ICLApp(n,args,"-shifts|-s(int=8,int=2,int=2) "
                "-seg-format|-f(format=YUV) "
                "[m]-input|-i(2) "
                "-num-classes|-n(int=12) "
                "-load|-l(filename) "
                "-reset-bus|-r",
                init,run).exec();
}
