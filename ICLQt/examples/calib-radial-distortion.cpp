#include <iclCommon.h>
#include <iclWarpOp.h>
#include <iclDynMatrix.h>
#include <iclMouseInteractionReceiver.h>

#include <QPushButton>
#include <iclLocalThresholdOp.h>
#include <iclRegionDetector.h>
#include <iclMorphologicalOp.h>
#include <iclCC.h>

#include "calib-radial-distortion-tools.h"
#include <iclSOM2D.h>
#include <iclMathematics.h>
#include <QMessageBox>
#include <QFileDialog>

GUI gui("hsplit");
GenericGrabber *grabber = 0;

Img32f IMAGE;
Mutex MUTEX;

CalibrationData CALIB_DATA;
ImgQ WARP_MAP;
double DIST_FACTOR[4];


void create_empty_warp_map(){
  const Size &size = grabber->getDesiredSize();
  WARP_MAP.setSize(size);
  WARP_MAP.setChannels(2);

  Channel32f cs[2];
  WARP_MAP.extractChannels(cs);
  
  for(int x=0;x<size.width;++x){
    for(int y=0;y<size.height;++y){
      cs[0](x,y) = x;
      cs[1](x,y) = y;
    }
  }
}

inline Point32f distort_point(int xi, int yi){
  const double &x0 = DIST_FACTOR[0];
  const double &y0 = DIST_FACTOR[1];
  const double &f = DIST_FACTOR[2]/100000000.0;
  const double &s = DIST_FACTOR[3];
  
  float x = s*(xi-x0);
  float y = s*(yi-y0);
  float p = 1 - f * (x*x + y*y);
  return Point32f(p*x + x0,p*y + y0);
}

void update_warp_map(){
  const Size &size = grabber->getDesiredSize();  
  Channel32f cs[2];
  WARP_MAP.extractChannels(cs);
  
  for(float xi=0;xi<size.width;++xi){
    for(float yi=0;yi<size.height; ++yi){
      Point32f p = distort_point(xi,yi);
      //      float x = s*(xi-x0);
      //float y = s*(yi-y0);
      //float p = 1 - f * (x*x + y*y);
      cs[0](xi,yi) = p.x; //p*x + x0; 
      cs[1](xi,yi) = p.y; //p*y + y0; 
      
      // csErr[0](xi,yi) = cs[0](xi,yi) - xi;
      // csErr[1](xi,yi) = cs[1](xi,yi) - yi;
    }
  }
}

void vis_som(SOM2D  &som, int gridW, int gridH){
  
  static DrawHandle &d = gui.getValue<DrawHandle>("image");
  static ICLDrawWidget &w = **d;
  ImgQ ps = zeros(gridW,gridH,2);
  
  w.lock();
  w.reset();
  w.color(255,0,0,255);
  for(int x=0;x<gridW;++x){
    for(int y=0;y<gridH;++y){
      float *p = som.getNeuron(x,y).prototype;
      ps(x,y,0) = p[0];
      ps(x,y,1) = p[1];
    }
  }
  for(int x=1;x<gridW;++x){
    for(int y=1;y<gridH;++y){
      w.line(ps(x,y,0),ps(x,y,1),ps(x-1,y,0),ps(x-1,y,1));
      w.line(ps(x,y,0),ps(x,y,1),ps(x,y-1,0),ps(x,y-1,1));
    }
  }
  for(int x=1;x<gridW;++x){
    w.line(ps(x,0,0),ps(x,0,1),ps(x-1,0,0),ps(x-1,0,1));
  }
  for(int y=1;y<gridH;++y){
    w.line(ps(0,y,0),ps(0,y,1),ps(0,y-1,0),ps(0,y-1,1));
  }
  w.unlock();
  d.update();
  Thread::msleep(1);
}

std::vector<Point32f> sort_points(const std::vector<Point32f> points, int gridW, int gridH, int imageW, int imageH){
  static std::vector<Range32f> initBounds(2,Range32f(0,1));
  static DrawHandle &d = gui.getValue<DrawHandle>("image");
  static ICLDrawWidget &w = **d;

  randomSeed();
  static const float M = 10;
  static const float E_start = 0.8;
  float mx = float(imageW-2*M)/(gridW-1);
  float my = float(imageH-2*M)/(gridH-1);
  float b = M;
  
  SOM2D som(2,gridW,gridH,initBounds,0.5,0.5);
  for(int x=0;x<gridW;++x){
    for(int y=0;y<gridH;++y){
      float *p = som.getNeuron(x,y).prototype;
      p[0] = mx * x + b;
      p[1] = my * y + b;
    }
  }

  URandI ridx(points.size()-1);
  float buf[2];

  std::vector<Point32f> sorted(points.size());  
  QMessageBox::StandardButton btn = QMessageBox::No;
  while(btn != QMessageBox::Yes){
    for(int j=0;j<100;++j){
      som.setEpsilon(E_start * ::exp(-j/30));
      for(int i=0;i<1000;++i){
        unsigned int ridxVal = ridx; 
        //      DEBUG_LOG("idx is " << ridxVal);
        const Point &rnd = points[ridxVal];
        buf[0] = rnd.x;
        buf[1] = rnd.y;
        som.train(buf);
      }
    }
    vis_som(som,gridW,gridH);

    w.lock();
    w.color(0,100,255,255);
    for(unsigned int i=0;i<points.size();++i){
      const Point &p = points[i];
      buf[0] = p.x;
      buf[1] = p.y;
      const float *g = som.getWinner(buf).gridpos;
      int x = ::round(g[0]);
      int y = ::round(g[1]);
      int idx = x + gridW * y;
      if(idx >=0 && idx < gridW*gridH){
        sorted.at(idx) = points.at(i);

        w.text(str(idx),p.x,p.y,-1,-1,12);
      }else{
        ERROR_LOG("could not sort point at index " << i);
      }
    }
    w.unlock();
    w.update();
    
    btn = QMessageBox::question(*gui.getValue<DrawHandle>("image"),
                                "please confirm ...","Is this tesselation correct?",
                                QMessageBox::Yes| QMessageBox::No);
  }
  return sorted;
}// -input file '~/tmp/images/*.ppm' -nx 5 -ny 4

void optimize_params(){
  Mutex::Locker l(MUTEX);
  
  Size size = grabber->getDesiredSize();
  calc_distortion(CALIB_DATA,size.width,size.height,DIST_FACTOR);
  
  std::cout << "distortion factors: {" << DIST_FACTOR[0] << ", " << DIST_FACTOR[1] 
            << ", " << DIST_FACTOR[2] << ", " << DIST_FACTOR[3]  << "}" << std::endl;

  update_warp_map();
}

void set_state(bool good){
  static ImgQ iGood(Size(75,25),formatRGB);
  static ImgQ iBad(Size(75,25),formatRGB);
  static bool first = true;
  if(first){
    first = false;
    color(0,255,0,255);
    fill(0,255,0,255);
    fontsize(12);
    text(iGood,20,0,"good");

    color(255,0,0,255);
    fill(255,0,0,255);
    text(iBad,2,0,"searching");
    
  }
  static ImageHandle &h = gui.getValue<ImageHandle>("state");
  h = good ? iGood : iBad;
  h.update();
}

void detect_vis(bool add=false){
  Mutex::Locker l(MUTEX);
  static DrawHandle &d = gui.getValue<DrawHandle>("image");
  static ICLDrawWidget &w = **d;

  static Img32f grayIm(IMAGE.getSize(),formatGray);
  cc(&IMAGE,&grayIm);
  
  static LocalThresholdOp lt(35,-10,0);
  static ImgBase *ltIm = 0;
  lt.apply(&grayIm,&ltIm);

  static std::vector<char> morphMask(9,1);
  static MorphologicalOp morph(Size(3,3),morphMask.data(),MorphologicalOp::dilate3x3);
  morph.setClipToROI(false);
  static ImgBase *moIm = 0;
  morph.apply(ltIm,&moIm);
  

  static RegionDetector rd(100,50000,0,0);
  const std::vector<icl::Region> &rs = rd.detect(moIm);
  
  static std::string &vis = gui.getValue<std::string>("vis");

  if(vis == "color"){
    d  = IMAGE;  
  }else if(vis == "gray"){
    d = grayIm;
  }else if(vis == "thresh"){
    d = ltIm;
  }else if(vis == "morph"){
    d = moIm;
  }else if(vis == "warp"){
    static WarpOp warp(WARP_MAP);
    static ImgBase *waIm = 0;
    warp.apply(&IMAGE,&waIm);
    d = waIm;
  }else if(vis == "warp-field"){
    static Img8u bg(IMAGE.getSize(),1);
    d = bg;
    w.lock();
    w.color(255,0,0,200);
    for(int x=IMAGE.getSize().width-10;x>=0;x-=20){
      for(int y=IMAGE.getSize().height-10;y>=0;y-=20){
        Point32f p = distort_point(x,y);
        w.line(x,y,p.x,p.y);
        w.sym(x,y,ICLDrawWidget::symCircle);
      }
    }
    w.unlock();
  }else if(vis == "warp-map"){
    d = WARP_MAP;
  }

  if(!add){
    w.lock();
    w.color(255,0,0,200);
  }
  std::vector<Point32f> pts;
  for(unsigned int i=0;i<rs.size();++i){
    static float &minFormFactor = gui.getValue<float>("min-form-factor");
    bool warpX = (vis == "warp") || (vis == "warp-field") || (vis == "warp-map");
    if(rs[i].getFormFactor() < minFormFactor){
      if(!add && !warpX){
        w.linestrip(rs[i].getBoundary());
        w.text(str(rs[i].getFormFactor()),rs[i].getCOG().x,rs[i].getCOG().y,-1,-1,10);
        w.sym(rs[i].getCOG().x,rs[i].getCOG().y,ICLDrawWidget::symPlus);
      }
      pts.push_back(rs[i].getCOG());
    }
  }
  if(!add){
    w.unlock();
    set_state((int)pts.size() == CALIB_DATA.dim());
  }else{
    DEBUG_LOG("adding data ...");
    if((int)pts.size() != CALIB_DATA.dim()){
      ERROR_LOG("unable to add current state: some markers are missing\n"
                "found " << pts.size() << " searching for " << CALIB_DATA.nx << "x" << CALIB_DATA.ny << "=" << CALIB_DATA.dim());
    }else{

      CALIB_DATA.data.push_back(CalibrationStep());
      IMAGE.deepCopy(&CALIB_DATA.data.back().colorImage);
      moIm->convert(&CALIB_DATA.data.back().image);
      CALIB_DATA.data.back().points = sort_points(pts,
                                                  CALIB_DATA.nx,
                                                  CALIB_DATA.ny,
                                                  grabber->getDesiredSize().width,
                                                  grabber->getDesiredSize().height);
    }
  }
}

void add(void){
  detect_vis(true);
}

void save_warp_map(){
  static DrawHandle &d = gui.getValue<DrawHandle>("image");
  
  std::string defName = str("./warp-map-")+translateSize(WARP_MAP.getSize())+
  "-"+str(DIST_FACTOR[0])+"-"+str(DIST_FACTOR[1])+"-"+str(DIST_FACTOR[2])+"-"+str(DIST_FACTOR[3])+".icl";
  QString name = QFileDialog::getSaveFileName(*d,"save warp-map ... ",defName.c_str(),
                                              "Float Images (*.icl *.pgm *.pnm)");
  if(name != ""){
    try{
      save(WARP_MAP,name.toLatin1().data());
    }catch(const std::exception &ex){
      ERROR_LOG("error while writing file ...");
    }
  }
}
void create_pattern_gui(){
  static ICLDrawWidget w;
  w.setGeometry(QRect(500,500,640,480));
  int W = CALIB_DATA.nx;
  int H = CALIB_DATA.ny;

  Img8u bg(Size(W+1,H+1),1);
  std::fill(bg.begin(0),bg.end(0),255);
  w.setImage(&bg);
  w.lock();
  w.reset();
  w.color(0,0,0,255);  
  w.fill(0,0,0,255);  
  
  float D = 0.4;
  for(int i=1;i<=W;++i){
    for(int j=1;j<=H;++j){
      w.ellipse(i,j,D,D);
    }
  }

  w.unlock();
  w.show();
}

void init(){
  CALIB_DATA.nx = pa_subarg<int>("-nx",0,5);
  CALIB_DATA.ny = pa_subarg<int>("-ny",0,4);

  if(pa_defined("-init")){
    for(int i=0;i<4;++i){
      DIST_FACTOR[0] = pa_subarg<float>("-init",i,0.0f);
    }
    update_warp_map();
  }
  
  if(pa_defined("-cp")){
    create_pattern_gui();
  }
 
  
  gui << "draw[@minsize=32x24@handle=image]";
  GUI controls("vbox[@minsize=10x1]");
  controls << "image[@maxsize=100x2@label=state@handle=state]";
  controls << "combo(!color,gray,thresh,morph,warp,warp-field,warp-map)[@out=vis@label=visualization]";
  controls << "togglebutton(off,on)[@out=grab-loop-val@handle=grab-loop@label=grab loop]";
  controls << "fslider(1.6,2.5,1.9)[@out=min-form-factor@label=roundness]";
  controls << "button(add)[@handle=add]";
  controls << "button(optimize)[@handle=optimize]";
  controls << "button(save)[@handle=save]";
  gui << controls;
  gui.show();
  
  //  gui.getValue<ButtonHandle>("detect").registerCallback(new GUI::Callback(detect));
  (*gui.getValue<ButtonHandle>("grab-loop"))->setChecked(true);

  //  (*gui.getValue<ImageHandle>("state"))->setFitMode(ICLWidget::fmFit);
  (*gui.getValue<ImageHandle>("state"))->setMenuEnabled(false);

  grabber = new GenericGrabber(FROM_PROGARG_DEF("-input","pwc","0"));
  grabber->setIgnoreDesiredParams(false);
  grabber->setDesiredSize(Size::VGA);
  grabber->setDesiredFormat(formatRGB);
  grabber->setDesiredDepth(depth32f);

  CALIB_DATA.data.reserve(10);

  gui.getValue<ButtonHandle>("add").registerCallback(new GUI::Callback(add));
  gui.getValue<ButtonHandle>("save").registerCallback(new GUI::Callback(save_warp_map));
  
  create_empty_warp_map();
}

void run(){

  static DrawHandle &d = gui.getValue<DrawHandle>("image");
  static ICLDrawWidget &w = **d;
  static bool &grab = gui.getValue<bool>("grab-loop-val");
  static ButtonHandle &add = gui.getValue<ButtonHandle>("add");
  static ButtonHandle &opt = gui.getValue<ButtonHandle>("optimize");

  if(grab){
    Mutex::Locker l(MUTEX);
    grabber->grab()->convert(&IMAGE);
  }
  
  w.lock();
  w.reset();
  w.unlock();
  
  detect_vis(false);
  
  d.update();

  if(opt.wasTriggered()){
    DEBUG_LOG("optimizing ...");
    optimize_params();
  }
  
  Thread::msleep(20);
}


int main(int n, char **ppc){
  pa_explain("-nx","count of marker grid points in horizontal direction (5 by default)");
  pa_explain("-ny","count of marker grid points in horizontal direction (4 by default)");
  pa_explain("-input","define input device (e.g. -dc 0 or -file 'images/*.ppm'");
  pa_explain("-init","defined 4 initial values for distortion factors");
  pa_explain("-cp","create an extra widget that shows a calibration pattern");
  pa_init(n,ppc,"-nx(1) -ny(1) -input(2) -cp -init(4)");
  
  ExecThread x(run);
  QApplication app(n,ppc);
  
  init();

  x.run();
  
  return app.exec();
}
