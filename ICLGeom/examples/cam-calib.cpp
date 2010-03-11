/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLGeom module of ICL                         **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#include <ICLQuick/Common.h>
#include <ICLGeom/Scene.h>

#include <ICLUtils/DynMatrix.h>
#include <ICLUtils/Point32f.h>
#include <ICLUtils/ConfigFile.h>

#include <ICLFilter/WarpOp.h>
#include <ICLFilter/LocalThresholdOp.h>
#include <ICLFilter/MorphologicalOp.h>
#include <ICLBlob/RegionDetector.h>

#include <ICLCC/CCFunctions.h>

#include <ICLCore/Mathematics.h>
#include <ICLUtils/StraightLine2D.h>

#include <QMessageBox>
#include <QFileDialog>
#include <QPushButton>

#include <fstream>

typedef FixedColVector<float,3> Vec3;

enum CAM_TYPE{ VIEW_CAM, CALIB_CAM};
GUI gui("hsplit"),sceneGUI("vbox");
Scene scene;
GenericGrabber *grabber = 0;
ImgParams imageParams;

static std::vector<float> compute_center_brightness(const std::vector<Point32f> &cogs, 
                                                    const std::vector<Rect> &bbs, 
                                                    const Img8u &maskedImage){
  // {{{ open

    std::vector<float> v(cogs.size());
    const Img8u &im = maskedImage; 
    const Channel8u &ci = im[0];
    Rect ir = im.getImageRect();
    const float cf = 0.3; // center fraction
    const float lr = (1.0 - cf)/2; // fraction for left and right of center
    for(unsigned int i=0;i<cogs.size();++i){
      Rect r = bbs[i] & ir;
      Rect rs(r.x+r.width*lr,r.y+r.height*lr,r.width*cf,r.height*cf);
      int buf = 0;
      for(int x=rs.x;x<rs.right();++x){
        for(int y=rs.y;y<rs.bottom();++y){
          buf += ci(x,y);
        }
      }
      int dim = rs.getSize().getDim();
      if(dim){
        v[i] = buf/dim;
      }
    }
    return v;
  }

  // }}}

struct IdxPoint32f{
  // {{{ open
  
  IdxPoint32f(){}
  IdxPoint32f(const std::vector<Point32f> *cogs,int idx, 
              const StraightLine2D *line, float centerBrightness):
    cogs(cogs),idx(idx),centerBrightness(centerBrightness){
    distToLine = line->signedDistance(StraightLine2D::Pos(p().x,p().y));
  }
  IdxPoint32f(const IdxPoint32f &ip,const StraightLine2D *otherLine):
    cogs(ip.cogs),idx(ip.idx),centerBrightness(ip.centerBrightness){
    distToLine = otherLine->signedDistance(StraightLine2D::Pos(p().x,p().y));
  }
  const std::vector<Point32f> *cogs;
  int idx;
  float distToLine;
  float centerBrightness;
  const Point32f &p() const { 
    return cogs->at(idx); 
  }

  bool operator<(const IdxPoint32f &p) const{
      return distToLine < p.distToLine;
  }
  static bool cmp_less(const IdxPoint32f &a, const IdxPoint32f &b){
      return a<b;
  }
  static bool cmp_greater(const IdxPoint32f &a, const IdxPoint32f &b){
    return !(a<b);
  }
};

// }}}
  
  
  
static std::vector<std::vector<IdxPoint32f> > partition_and_sort_points(const std::vector<IdxPoint32f> &all, 
                                                                        int ny, const StraightLine2D *line,
                                                                        bool sortReverse){
  // {{{ open

  std::vector<std::vector<IdxPoint32f> > columns(all.size() / ny);
  std::vector<IdxPoint32f>::const_iterator it = all.begin();
  for(unsigned int i=0;i<columns.size();++i){
    columns[i].resize(ny);
    for(int j=0;j<ny;++j){
      columns[i][j] = IdxPoint32f(*it++, line);
    }
    std::sort(columns[i].begin(),columns[i].end(),IdxPoint32f::cmp_less);
    
  }
  if(sortReverse){
    std::reverse(columns.begin(),columns.end());
  }
  return columns;
}

// }}}
  
static std::pair<int,int> count_distance_signs(const std::vector<IdxPoint32f> &cogsi){
  // {{{ open

  std::pair<int,int> ds(0,0);
  for(unsigned int i=0;i<cogsi.size();++i){
    if(fabs(cogsi[i].distToLine) > 0.1){
      if(cogsi[i].distToLine>0) ds.second++;
      else ds.first++;
    }
  }
  return ds;
}

// }}}
    
struct CalibGrid {
  // {{{ open

  struct Half{
    // {{{ open

    std::vector<Point32f> img;  // row-major order
    std::vector<Vec3> world;     // row-major order
    Point32f p1,p2,p3;
  };

  // }}}

  int nx,ny;
  Half A,B;
  bool inputDataReady;
  
  CalibGrid():inputDataReady(false){}
  
  void visualizeTo(ICLDrawWidget &w){
    // {{{ open
    if(!inputDataReady) return;

    w.color(0,100,255,255);
    w.linewidth(1);
    w.grid(A.img.data(),nx,ny);
    w.linewidth(3);
    w.color(0,100,255,200);
    w.arrow(A.p1,A.p2);
    w.arrow(A.p1,A.p3);
    
    w.color(255,0,0,255);
    w.linewidth(1);
    w.grid(B.img.data(),nx,ny);
    w.linewidth(3);
    w.color(255,0,0,200);
    w.arrow(B.p1,B.p2);
    w.arrow(B.p1,B.p3);
    w.linewidth(1);

    w.color(200,0,255,255);
    
#if 0
    for(int x=0;x<nx;++x){
      for(int y=0;y<ny;++y){
        int i = x+nx*y;
        w.text(str(A.world[i].transp()),A.img[i].x,A.img[i].y,-1,-1,8);
        w.text(str(B.world[i].transp()),B.img[i].x,B.img[i].y,-1,-1,8);
      }
    }
#endif
  }
  // }}}
  
  
  CalibGrid(const std::string &configFileName) : inputDataReady(false){
    // {{{ open

    ConfigFile f(configFileName);
    Vec3 wo = parse<Vec3>(f["config.world-offset"]);
    nx = f["config.calibration-object.nx"];
    ny = f["config.calibration-object.ny"];
    
    f.setPrefix("config.calibration-object.part-A.");
    
    Vec3 A1 = wo + parse<Vec3>(f["offset"]);
    Vec3 adx = parse<Vec3>(f["dx"])/(nx-1);
    Vec3 ady = parse<Vec3>(f["dy"])/(ny-1);

    f.setPrefix("config.calibration-object.part-B.");
    
    Vec3 B1 = wo + parse<Vec3>(f["offset"]);
    Vec3 bdx = parse<Vec3>(f["dx"])/(nx-1);
    Vec3 bdy =  parse<Vec3>(f["dy"])/(ny-1);

    for(int y=0;y<ny;++y){    
      for(int x=0;x<nx;++x){
        A.world.push_back( A1 + adx * (nx-1-x) + ady * (ny-1-y));
        B.world.push_back( B1 + bdx * (   x  ) + bdy * (ny-1-y));
      }
    }
  }

  // }}}
  
  void update(const std::vector<Point32f> &cogs, const std::vector<Rect> &bbs, const Img8u &maskedImage){
    // {{{ open
    if((int)cogs.size() != 2*nx*ny || (int)bbs.size() != 2*nx*ny){
      inputDataReady = false;
      return;
    }
    std::vector<float> cbs = compute_center_brightness(cogs,bbs,maskedImage);
    int s1 = int(std::max_element(cbs.begin(),cbs.end()) - cbs.begin());
    float cbs_1 = cbs[s1];
    cbs[s1] = 0;
    int s2 = int(std::max_element(cbs.begin(),cbs.end()) - cbs.begin());
    cbs[s1] = cbs_1;
    
    StraightLine2D line(StraightLine2D::Pos(cogs[s1].x,cogs[s1].y),
                        StraightLine2D::Pos(cogs[s2].x,cogs[s2].y)-
                        StraightLine2D::Pos(cogs[s1].x,cogs[s1].y));
    
    
    
    std::vector<IdxPoint32f> cogsi(cogs.size());
    for(unsigned int i=0;i<cogs.size();++i){
      cogsi[i] = IdxPoint32f(&cogs,i,&line, cbs[i]);
    }
    std::sort(cogsi.begin(),cogsi.end());
    std::pair<int,int> ds = count_distance_signs(cogsi);
    StraightLine2D perpLine = line;
    
    if(ds.first > ds.second){
      perpLine.v = StraightLine2D::Pos(line.v[1], -line.v[0]);
    }else{
      perpLine.v = StraightLine2D::Pos(-line.v[1], line.v[0]);
    }
    std::vector<std::vector<IdxPoint32f> > columns = partition_and_sort_points(cogsi, ny, &perpLine,
                                                                               ds.first > ds.second);
    A.img.resize(nx*ny);
    B.img.resize(nx*ny);
    for(int i=0;i<nx;++i){
      for(int j=0;j<ny;++j){
        A.img[i+nx*j] = columns[i][j].p();
        B.img[i+nx*j] = columns[i+nx][j].p();
      }
    }
    A.p1 = A.img.back();
    A.p2 = A.img[(ny-1)*nx];
    A.p3 = A.img[nx-1];

    B.p1 = B.img[(ny-1)*nx];
    B.p2 = B.img.back();
    B.p3 = B.img.front();

    inputDataReady = true;
  }

  // }}}

  struct flip_points_xy{
    // {{{ open

    Size size;
    flip_points_xy(const Size &size):size(size){}
    inline Point32f operator()(const Point32f &p){
      return Point32f(size.width-p.x,size.height-p.y);
    }
  };

  // }}}

  static Vec vec3to4(const Vec3 &v){
    // {{{ open

    return v.resize<1,4>(1);
  }

  // }}}

  float getRMSEOnImage(const std::vector<Vec> &ws, const std::vector<Point32f> &is, const Camera &cam) {
    // {{{ open

  float result = 0;
  for (unsigned int i=0; i<is.size(); i++) {
    result += pow(cam.project(ws[i]).distanceTo(is[i]),2);
  }
  return sqrt(result/is.size());
}

// }}}
  
  float applyCalib(){
    // {{{ open
    if(!inputDataReady) return Range32f::limits().maxVal;
    // create appropriate point sets
    std::vector<Point32f> imgPts;
    //std::transform(A.img.begin(),A.img.end(),back_inserter(imgPts),flip_points_xy(imageParams.getSize()));
    //std::transform(B.img.begin(),B.img.end(),back_inserter(imgPts),flip_points_xy(imageParams.getSize()));
    std::copy(A.img.begin(),A.img.end(),back_inserter(imgPts));
    std::copy(B.img.begin(),B.img.end(),back_inserter(imgPts));

    std::vector<Vec> worldPts;
    std::transform(A.world.begin(),A.world.end(),back_inserter(worldPts),vec3to4);
    std::transform(B.world.begin(),B.world.end(),back_inserter(worldPts),vec3to4);

    Camera cam = Camera::calibrate(worldPts, imgPts);
    cam.getRenderParams().viewport = Rect(Point::null,imageParams.getSize());
    cam.getRenderParams().chipSize = imageParams.getSize();
    scene.getCamera(CALIB_CAM) = cam;
    return getRMSEOnImage(worldPts,imgPts, cam);
  }
  // }}}

} calibGrid;

// }}}

struct MaskRect{
  // {{{ open

  Mutex mutex;
  Point origin;
  Point curr;
  Rect rect;
  Img8u maskedImage;
  void draw(ICLDrawWidget &w, int imagew, int imageh){
    // {{{ open

    Mutex::Locker l(mutex);
    if(rect != Rect::null ){
      w.color(0,100,255,255);
      w.fill(0,0,0,0);
      w.rect(rect.x,rect.y,rect.width,rect.height);

      w.color(0,0,0,0);
      w.fill(0,100,255,150);
      w.rect(0,0,rect.x,imageh);
      w.rect(rect.right(),0,imagew-rect.right(),imageh);
      w.rect(rect.x,rect.bottom(),rect.width,imageh-rect.bottom());
      w.rect(rect.x,0,rect.width,rect.y);
    }
    if(curr != Point::null && origin != Point::null){
      w.color(0,255,40,255);
      w.fill(0,0,0,0);
      Rect r(origin, Size(curr.x-origin.x,curr.y-origin.y ));
      w.rect(r.x,r.y,r.width,r.height);
    }
  }

  // }}}
  const Img8u &applyMask(const Img8u &src){
    // {{{ open

    Mutex::Locker l(mutex);
    if(rect == Rect::null) return src;
    maskedImage.setParams(src.getParams());
    maskedImage.clear();
    Img8u srcCpy(src);
    srcCpy.setROI(rect & srcCpy.getImageRect());
    maskedImage.setROI(rect & srcCpy.getImageRect());
    srcCpy.deepCopyROI(&maskedImage);
    return maskedImage;
  }

  // }}}
} maskRect;

// }}}

struct MaskRectMouseHandler : public MouseHandler{
  // {{{ open

  virtual void process(const MouseEvent &e){
    Mutex::Locker l(maskRect.mutex);
    if(e.isRight()){
      maskRect.rect = Rect::null;
      return;
    }
    if(e.isPressEvent()){
      maskRect.origin  = e.getPos();
      maskRect.curr   = e.getPos();
    }else if(e.isDragEvent()){
      maskRect.curr   = e.getPos();
    }
    if(e.isReleaseEvent()){
      if(maskRect.origin != maskRect.curr){
        maskRect.rect = Rect(maskRect.origin, Size(maskRect.curr.x-maskRect.origin.x,
                                                   maskRect.curr.y-maskRect.origin.y ));
        maskRect.rect = maskRect.rect.normalized();
        if(maskRect.rect.getDim() < 4) {
          maskRect.rect = Rect::null;
        }
      }
      maskRect.origin = Point::null;
      maskRect.curr = Point::null;
    }
  }
};

// }}}





void show_hide_scene(){
  // {{{ open
  sceneGUI.switchVisibility();
}

// }}}

void add_cuboid(float a, float b, float c, float d, float e, float f, const GeomColor &col){
  // {{{ open

  const float p[] = { a,b,c,d,e,f };
  SceneObject *obj = new SceneObject("cuboid",p);
  obj->setColor(Primitive::quad,col);
  obj->setVisible(Primitive::line,false);
  scene.addObject(obj);
}

// }}}

void init_scene_and_scene_gui(){
  // {{{ open
  
  std::string size = str(imageParams.getSize()/20);
  sceneGUI << (GUI("hbox") 
               << "button(sync)[@handle=syncCams]"
               << "checkbox(visualize cams,off)[@out=visCams]")
           << (GUI("tab(real camera,free camera)[@size=32x24]")
               << "draw3D[@handle=calibScene@minsize=16x12]"
               << "draw3D[@minsize=16x12@handle=scene]");
  sceneGUI.create();

  scene.addCamera(Camera(Vec(-500,-320,570),
                         Vec(0.879399,0.169548,-0.444871),
                         Vec(0.339963,0.263626,0.902733)));
  scene.addCamera(Camera());

  ICLDrawWidget3D &w = **sceneGUI.getValue<DrawHandle3D>("scene");
  w.setImageInfoIndicatorEnabled(false);
  ICLDrawWidget3D &cw = **sceneGUI.getValue<DrawHandle3D>("calibScene");
  cw.setImageInfoIndicatorEnabled(false);

  static const float s = 3;
  static const float l = 200;
  static const float l2 = l/2;
  add_cuboid(0,0,0,s,s,s,GeomColor(255,255,0,255));
  add_cuboid(l2,0,0,l,s,s,GeomColor(255,0,0,255));
  add_cuboid(0,l2,0,s,l,s,GeomColor(0,255,0,255));
  add_cuboid(0,0,l2,s,s,l,GeomColor(0,100,255,255));

  // add calibration pattern
  for(unsigned int i=0;i<calibGrid.A.world.size();++i){
    const Vec3 &pa = calibGrid.A.world[i];
    add_cuboid(pa[0],pa[1],pa[2],7,7,7,GeomColor(0,100,255,255));
    const Vec3 &pb = calibGrid.B.world[i];
    add_cuboid(pb[0],pb[1],pb[2],7,7,7,GeomColor(255,0,0,255));
  }

  w.install(scene.getMouseHandler(VIEW_CAM));  
  cw.install(scene.getMouseHandler(CALIB_CAM));  
}

// }}}

void run_scene(){
  // {{{ open
  GUI &gui=sceneGUI;
  gui_ButtonHandle(syncCams);
  gui_bool(visCams);
  
  scene.setDrawCamerasEnabled(visCams);
  
  if(syncCams.wasTriggered()){
    scene.getCamera(VIEW_CAM) = scene.getCamera(CALIB_CAM);
  }
  
  static ICLDrawWidget3D *ws[2] = {
    *sceneGUI.getValue<DrawHandle3D>("scene"),
    *sceneGUI.getValue<DrawHandle3D>("calibScene")
  };

  for(int i=0;i<2;++i){
    ws[i]->lock();

    ws[i]->reset3D();
    ws[i]->callback(scene.getGLCallback(i));
    ws[i]->unlock();
    ws[i]->updateFromOtherThread();
  }
}

// }}}

void init(){
  // {{{ open
#define X(x) << x << std::endl
  if(pa("-create-empty-config-file")){
    std::ofstream of("new-calib-config.xml");
    of << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>" << std::endl
    X("<config>")
    X("<!-- Layout:")
    X("0 -- 1 -- 2 -- 3  |  16 - 17 - 18 - 19")
    X("4 -- 5 -- 6 -- 7  |  20 - 21 - 22 - 23")
    X("8 -- 9 - 10 - 11  |  24 - 25 - 26 - 27")
    X("12 - 13 - 14 - 15  |  28 - 29 - 30 - 31")
    X("                                                  ___")
    X(" a------a------a------A3 B3-----b-----b-----b      |")
    X(" |      |      |      |   |     |     |     |      |")
    X(" a------a------a------a   b-----b-----b-----b      | ")
    X(" |      |      |      |   |     |     |     |     ny (cells)")
    X(" a------a------a------a   b-----b-----b-----b      |")
    X(" |      |      |      |   |     |     |     |      |")
    X("A2------a------a------A1 B1-----b-----b-----B2    _|_")
    X(" ")
    X(" |-------- nx --------|")
    X("         (cells)  ")
    X(" ")
    X("A1 = partA.offset")
    X("A2 = partA.dx + partA.offset")
    X(" A3 = partA.dy + partA.offset")
    X(" ")
    X("B1 = partB.offset")
    X(" B2 = partB.dx + partB.offset")
    X("B3 = partB.dy + partB.offset")
    X("-->")
    X("<title>Camera Calibration Config-File</title>")
    X("<data id='world-offset' type='string'>0,266,0</data>")
    X("<section id='calibration-object'>")
    X("  <data id='nx' type='int'>5</data>")
    X("  <data id='ny' type='int'>4</data>")
    X("  <section id='part-A'>")
    X("    <data id='offset' type='string'>-21.21 30.8 282.29</data>")
    X("    <data id='dx' type='string'>-200.12 0 -200.12</data>")
    X("    <data id='dy' type='string'>0 212.25 0</data>")
    X("  </section>")
    X("  <section id='part-B'>")
    X("    <data id='offset' type='string'>21.92 29.5 281.58</data>")
    X("    <data id='dx' type='string'>200.12 0 -200.12</data>")
    X("    <data id='dy' type='string'>0 212.25 0</data>")
    X("  </section>")
    X("</section>")
    X("</config>");
    exit(0);
  }

  calibGrid = CalibGrid(*pa("-c"));

  gui << "draw[@minsize=16x12@label=main view@handle=mainview]";

  GUI controls("vbox");
  controls << ( GUI("hbox") 
                <<  "combo(!color,gray,thresh)[@out=vis-val@handle=vis@label=visualization]"
                << "togglebutton(off,!on)[@out=grab@handle=grab-loop@label=grab loop]"
                << "checkbox(morph,off)[@out=morphImageOn]");

  
  controls << ( GUI("hbox") 
                << "togglebutton(off,!on)[@out=visPoints@handle=vis-overlay@label=show points]"
                << "togglebutton(off,!on)[@out=visGrid@handle=ass-vis-on@label=show grid]");
                
  controls << ( GUI("hbox") 
                << "togglebutton(off,!on)[@out=calibOn@handle=calibrate-on@label=calibration]"
                << "label()[@label=current error: @handle=currError]" );
  
  controls << "fslider(0.6,2 .0,1.5)[@out=minFF@label=roundness]";
  controls << "slider(10,5000,90)[@out=minBlobSize@label=min blob size]";
  controls << "slider(10,10000,1000)[@out=maxBlobSize@label=max blob size]";
  controls << (GUI("vbox[@label=local threshold]") 
               << "slider(2,100,10)[@out=ltMaskSize@label=mask size]"
               << "slider(-20,20,-10)[@out=ltThresh@label=threshold]");
  controls << (GUI("hbox") 
               << "button(show/hide scene)[@handle=showScene]"
               << "button(print camera)[@handle=printCam]"
               << "button(save best of 10)[@handle=saveBest10]"
               );

  gui << controls;
  
  gui.show();
  
  gui.registerCallback(new GUI::Callback(show_hide_scene),"showScene");
  gui["mainview"].install(new MaskRectMouseHandler);

  grabber = new GenericGrabber(FROM_PROGARG("-input"));
  grabber->setIgnoreDesiredParams(true);
  imageParams = grabber->grab()->getParams();
  if(pa("-dist")){
    grabber->enableDistortion(DIST_FROM_PROGARG("-dist"),imageParams.getSize());
  }

  init_scene_and_scene_gui();
}

// }}}

void run(){
  // {{{ open
  
  static int framesLeft = -1;
  
  gui_DrawHandle(mainview);
  gui_ComboHandle(vis);
  gui_ButtonHandle(printCam);
  gui_ButtonHandle(saveBest10);
  gui_LabelHandle(currError);

  gui_bool(grab);
  gui_bool(calibOn);
  gui_bool(morphImageOn);
  gui_bool(visPoints);
  gui_bool(visGrid);
  
  gui_int(minBlobSize);
  gui_int(maxBlobSize);
  gui_int(ltThresh);
  gui_int(ltMaskSize);
  gui_float(minFF);

  GUI &gui = sceneGUI;
  gui_DrawHandle3D(calibScene);

  
  static Img8u image;
  if(grab || !image.getDim()){
    grabber->grab()->convert(&image);
  }
  
  static Img8u grayIm(image.getSize(),formatGray);
  cc(&image,&grayIm);
  
  static LocalThresholdOp lt(35,-10,0);

  lt.setGlobalThreshold(ltThresh);
  lt.setMaskSize(ltMaskSize);

  const ImgBase *ltIm = lt.apply(&grayIm);

  if(morphImageOn){
    static MorphologicalOp morph(MorphologicalOp::dilate3x3);
    morph.setClipToROI(false);
    ltIm = morph.apply(ltIm);
  }

  const Img8u &maskedImage = maskRect.applyMask(*ltIm->asImg<icl8u>());

  static RegionDetector rd(100,50000,0,0);
  rd.setRestrictions(minBlobSize,maxBlobSize,0,0);

  const std::vector<icl::Region> &rsd = rd.detect(&maskedImage);
  std::vector<icl::Region> rs;
  std::vector<Point32f> cogs;
  std::vector<Rect> bbs;
  std::vector<Point32f> accurate_cogs;
  for(unsigned int i=0;i<rsd.size();++i){
    
    if(rsd[i].getFormFactor() <= minFF){
      rs.push_back(rsd[i]);
      cogs.push_back(rsd[i].getCOG());
      bbs.push_back(rsd[i].getBoundingBox());
      accurate_cogs.push_back(rsd[i].getAccurateCOG(grayIm));
    }
  }
  

  calibGrid.update(cogs,bbs,grayIm);


  if(vis.getSelectedItem() == "color"){
    mainview = image;
  }else if (vis.getSelectedItem() == "gray"){
    mainview = grayIm;
  }else if (vis.getSelectedItem() == "thresh"){
    mainview = ltIm;
  }

  float lastError = Range32f::limits().maxVal;
  if(calibOn){
    lastError = calibGrid.applyCalib();
    currError = lastError;
  }else{
    currError = "---";
  }
  
  
  ICLDrawWidget &w = **mainview;
  w.lock();
  w.reset();
  maskRect.draw(w,w.getImageSize().width,w.getImageSize().height);
  
  if(visPoints){
    w.color(255,0,0);
    w.fill(255,0,0,50);
    w.symsize(10);
    for(unsigned int i=0;i<rs.size();++i){
      const icl::Region &r = rs[i];
      w.color(255,0,0);
      w.rect(r.getBoundingBox());
      w.sym(cogs[i].x,cogs[i].y,ICLDrawWidget::symPlus);
      w.color(0,255,0);
      w.sym(accurate_cogs[i].x,accurate_cogs[i].y,ICLDrawWidget::symCross);
    }
    
  }
  if(visGrid){
    w.color(255,0,0);
    calibGrid.visualizeTo(w);
  }

  w.unlock();
  mainview.update();

  if(sceneGUI.getRootWidget()->isVisible()){
    calibScene = &image;
    run_scene();
  }
  

  if(printCam.wasTriggered()){
    Camera c = scene.getCamera(CALIB_CAM);
    std::cout << "------------------------------------------------------" << std::endl;
    DEBUG_LOG("estimated camera pos is:" << c.getPosition());
    //c.setPosition(c.getPosition()+worldOffset);
    std::string filename = pa("-o");
    std::cout << "new config file: (written to " <<  filename << ")" << std::endl;
    std::cout << c << std::endl;
    
    //std::ofstream file(filename.c_str());
    //file << c;
    std::cout << "------------------------------------------------------" << std::endl;
  }

  
  if(framesLeft != -1 || saveBest10.wasTriggered()){
    static Camera cams[10];
    static float errs[10];

    if(framesLeft < 0){
      framesLeft = 9;
      cams[framesLeft] = scene.getCamera(CALIB_CAM);
      errs[framesLeft] = lastError;
      framesLeft--;
      std::cout << "captured frame " << (10-framesLeft) << "/10" << std::endl;
    }else if(framesLeft > 0){
      cams[framesLeft] = scene.getCamera(CALIB_CAM);
      errs[framesLeft] = lastError;
      framesLeft--;
      std::cout << "captured frame " << (10-framesLeft) << "/10" << std::endl;
    }else if(framesLeft == 0){
      framesLeft = -1;
      cams[0] = scene.getCamera(CALIB_CAM);
      errs[0] = lastError;
      std::cout << "captured frame " << (10-framesLeft) << "/10" << std::endl;
      int idx = (int)(std::min_element(errs,errs+10)-errs);
      if(errs[0] == Range32f::limits().maxVal){
        ERROR_LOG("Invalid calibration (nothing saved)");
      }else{
        Camera c = cams[idx];
        std::cout << "------------------------------------------------------" << std::endl;
        std::cout << "estimated camera pos is:" << c.getPosition().transp() <<std::endl;
        //std::cout << "worldOffset is:" << worldOffset.transp() << std::endl;
        std::cout << "best calibration error was:" << errs[idx] << std::endl << std::endl;
        //c.setPosition(c.getPosition()+worldOffset);
        std::string filename = pa("-o");
        std::cout << "new config file: (written to " <<  filename << ")" << std::endl;
        std::cout << c << std::endl;
        
        std::ofstream file(filename.c_str());
        file << c;
        std::cout << "------------------------------------------------------" << std::endl;
      }
    }
  }

  Thread::msleep(10);
}

// }}}

int main(int n, char **ppc){
  // {{{ open
  paex
  ("-input","define input device e.g. '-input dc 0' or '-input file *.ppm'")
  ("-o","define output config xml file name (./extracted-camera-cfg.xml)")
  ("-config","define input marker config file (calib-config.xml by default)")
  ("-dist","give 4 distortion parameters")
  ("-create-empty-config-file","if this flag is given, an empty config file is created as ./new-calib-config.xml");
  return ICLApp(n,ppc,"[m]-input|-i(device,device-params) "
                "-config|-c(config-xml-file-name=calib-config.xml) "
                "-dist|d(float,float,float,float) "
                "-create-empty-config-file|-cc "
                "-output|-o(output-xml-file-name=extracted-cam-cfg.xml)",init,run).exec();
}

// }}}
