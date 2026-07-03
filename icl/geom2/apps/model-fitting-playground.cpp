// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Model-fitting method playground. Two tabs, both driven by the icl::math::fit
// framework:
//   • "2D primitives" — pick a method (algebraic / Taubin / RANSAC-MSAC / trimmed /
//     seeded-geometric circle; algebraic / RANSAC line), tune it live via the
//     auto-generated Prop panel, watch the fit + inliers on a 2D Plot.
//   • "superquadric"  — fit a 3D superquadric's size + squareness to noisy surface
//     points with CMAESOptimizer (minimising the inside-outside error); points +
//     fitted surface render on a 3D Plot3D.
// Analogous to icl-filter-playground.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/qt/BoxHandle.h>
#include <icl/geom2/Plot3D.h>
#include <icl/math/fit/PrimitiveFitters2D.h>
#include <icl/math/fit/RobustFitter.h>
#include <icl/math/fit/RefiningFitter.h>
#include <icl/math/fit/GeometricRefiners2D.h>
#include <icl/math/fit/CMAESOptimizer.h>
#include <icl/utils/Random.h>
#include <icl/utils/Point.h>
#include <memory>
#include <mutex>
#include <iomanip>
#include <sstream>
#include <cmath>

using namespace icl::math;
using namespace icl::geom2;
using namespace icl::geom;

using Model = std::vector<double>;
using MF    = ModelFitter<Point32f, Model>;
using RF    = RobustFitter<Point32f, Model>;
using RefF  = RefiningFitter<Point32f, Model>;

GUI gui;

// =====================================================================
//  Tab 1 — 2D primitive fitting
// =====================================================================
enum Shape { LINE, CIRCLE };

struct Method {
  std::unique_ptr<MF>   top;                 // fit() target + Prop root
  std::unique_ptr<MF>   sub;                 // base/seed for robust/seeded
  std::unique_ptr<RefF> refiner;             // for seeded
  RF                   *robust = nullptr;    // non-owning; set when top is robust
  Shape                 shape  = CIRCLE;
};

static const std::vector<std::string> METHOD_NAMES = {
  "circle: algebraic (Kasa)", "circle: Taubin", "circle: RANSAC/MSAC",
  "circle: Taubin + RANSAC", "circle: algebraic + geometric refine",
  "line: algebraic", "line: RANSAC/MSAC",
};

static Method makeMethod(const std::string &name){
  Method m;
  if(name == "circle: algebraic (Kasa)"){ m.shape=CIRCLE; m.top.reset(new CircleFitter2D); }
  else if(name == "circle: Taubin"){ m.shape=CIRCLE; m.top.reset(new TaubinCircleFitter); }
  else if(name == "circle: RANSAC/MSAC"){ m.shape=CIRCLE; m.sub.reset(new CircleFitter2D);
    RF *r=new RF(m.sub.get(),0.05,0.99,1000,"trimmed",true,0.7); m.robust=r; m.top.reset(r); }
  else if(name == "circle: Taubin + RANSAC"){ m.shape=CIRCLE; m.sub.reset(new TaubinCircleFitter);
    RF *r=new RF(m.sub.get(),0.05,0.99,1000,"trimmed",true,0.7); m.robust=r; m.top.reset(r); }
  else if(name == "circle: algebraic + geometric refine"){ m.shape=CIRCLE; m.sub.reset(new CircleFitter2D);
    m.refiner.reset(new GeometricCircleRefiner);
    m.top.reset(new SeededFitter<Point32f,Model>(m.sub.get(), m.refiner.get())); }
  else if(name == "line: algebraic"){ m.shape=LINE; m.top.reset(new LineFitter2D); }
  else if(name == "line: RANSAC/MSAC"){ m.shape=LINE; m.sub.reset(new LineFitter2D);
    RF *r=new RF(m.sub.get(),0.3,0.99,3000,"trimmed",true,0.7); m.robust=r; m.top.reset(r); }
  return m;
}

static std::recursive_mutex g_mutex;
static Method               g_method;
static std::vector<Point32f> g_points;
static const float LINE_M=0.6f, LINE_B=0.4f, CX=1.2f, CY=0.5f, R=1.5f;

static void regenerate(){
  std::scoped_lock lock(g_mutex);
  const int num=gui["num"], outPct=gui["outliers"]; const float noise=gui["noise"];
  const int good = num*(100-outPct)/100;
  g_points.clear(); g_points.reserve(num);
  utils::GRand gn(0, noise);
  if(g_method.shape==CIRCLE){
    utils::URand ang(0,2*M_PI), box(-1.5,4.0);
    for(int i=0;i<good;++i){ const double a=ang;
      g_points.push_back(Point32f(CX+R*std::cos(a)+(float)gn, CY+R*std::sin(a)+(float)gn)); }
    for(int i=good;i<num;++i) g_points.push_back(Point32f(box,box));
  }else{
    utils::URand x(-3,3), oy(-3,3);
    for(int i=0;i<good;++i){ const float xx=x; g_points.push_back(Point32f(xx, LINE_M*xx+LINE_B+(float)gn)); }
    for(int i=good;i<num;++i){ const float xx=x; g_points.push_back(Point32f(xx, oy)); }
  }
}

static double geomResidual(Shape s, const Model &m, const Point32f &p){
  if(s==CIRCLE){ const double a=m[0], cx=-m[1]/(2*a), cy=-m[2]/(2*a);
    const double r=std::sqrt(std::max(0.0,(m[1]*m[1]+m[2]*m[2])/(4*a*a)-m[3]/a));
    return std::abs(std::hypot(p.x-cx,p.y-cy)-r); }
  return std::abs(m[0]*p.x+m[1]*p.y+m[2])/std::hypot(m[0],m[1]);
}

static void drawModel(PlotHandle &plot, Shape s, const Model &m){
  if(!m.size()) return;
  if(s==CIRCLE){ const double a=m[0];
    float c[3]={ float(-m[1]/(2*a)), float(-m[2]/(2*a)),
                 float(std::sqrt(std::max(0.0,(m[1]*m[1]+m[2]*m[2])/(4*a*a)-m[3]/a))) };
    plot->addAnnotations('c', c, 1, QColor(0,120,255));
  }else{ auto y=[&](float x){ return (-m[2]-m[0]*x)/m[1]; };
    float seg[4]={-3.f,(float)y(-3.f),3.f,(float)y(3.f)};
    plot->addAnnotations('l', seg, 1, QColor(0,120,255)); }
}

static void compute2D(){
  std::scoped_lock lock(g_mutex);
  if(!g_method.top || g_points.empty()) return;
  const Time t0=Time::now();
  Model m = g_method.top->fit(g_points);
  const double ms=(Time::now()-t0).toMicroSecondsDouble()/1000.0;

  PlotHandle plot = gui["plot2d"];
  plot->lock(); plot->clear();
  plot->setDataViewPort(g_method.shape==CIRCLE?Range32f(-1.6,4.1):Range32f(-3.2,3.2),
                        g_method.shape==CIRCLE?Range32f(-1.6,4.1):Range32f(-3.2,3.2));
  std::vector<float> xs,ys; for(const auto&p:g_points){ xs.push_back(p.x); ys.push_back(p.y); }
  plot->addScatterData('.', xs.data(), ys.data(), (int)xs.size(), "points", 230,60,60, 4);
  int nin=-1;
  if(g_method.robust){ const auto&in=g_method.robust->inliers(); nin=(int)in.size();
    std::vector<float> ix,iy; for(const auto&p:in){ ix.push_back(p.x); iy.push_back(p.y); }
    if(nin) plot->addScatterData('.', ix.data(), iy.data(), nin, "inliers", 40,200,40, 4); }
  drawModel(plot, g_method.shape, m);
  plot->unlock(); plot.render();

  double sse=0; int cnt=0;
  auto acc=[&](const Point32f&p){ const double e=geomResidual(g_method.shape,m,p); sse+=e*e; ++cnt; };
  if(g_method.robust && nin>0) for(const auto&p:g_method.robust->inliers()) acc(p);
  else for(const auto&p:g_points) acc(p);
  std::ostringstream st; st<<"RMS "<<std::fixed<<std::setprecision(4)<<(cnt?std::sqrt(sse/cnt):0)<<" | ";
  if(nin>=0) st<<nin<<"/"<<g_points.size()<<" inliers | ";
  st<<std::setprecision(2)<<ms<<" ms";
  gui["stats2d"] = st.str();
}

static void rebuildProps(const std::string &name){
  static GUI propGUI;
  std::scoped_lock lock(g_mutex);
  BoxHandle box = gui.get<BoxHandle>("props2d");
  if(propGUI.hasBeenCreated()) propGUI.hide();
  g_method = makeMethod(name); regenerate();
  propGUI = GUI(VBox());
  if(g_method.top) propGUI << Prop(g_method.top.get(), {.label=name});
  propGUI.create(); box.add(propGUI.getRootWidget());
}

// =====================================================================
//  Tab 2 — superquadric fitting (CMA-ES)
// =====================================================================
static std::recursive_mutex sq_mutex;
static CMAESOptimizer<std::vector<double>> sq_opt(1500, 0.4, 1e-12);

static inline double sgnpow(double b, double e){ return (b<0?-1.0:1.0)*std::pow(std::abs(b), e); }

// inside-outside value F(x,y,z) of an axis-aligned, origin-centred superquadric
static inline double sqInsideOut(double x,double y,double z, double a,double b,double c,double e1,double e2){
  return std::pow(std::pow(std::abs(x/a),2/e2)+std::pow(std::abs(y/b),2/e2), e2/e1)
       + std::pow(std::abs(z/c),2/e1);
}

// nx*ny grid of surface points (row-major) for the given SQ params
static std::vector<Vec> sqSurface(double a,double b,double c,double e1,double e2,int nx,int ny){
  std::vector<Vec> g; g.reserve(nx*ny);
  for(int i=0;i<nx;++i){ const double eta=-M_PI/2 + M_PI*i/(nx-1);
    for(int j=0;j<ny;++j){ const double om=-M_PI + 2*M_PI*j/(ny-1);
      g.push_back(Vec(a*sgnpow(std::cos(eta),e1)*sgnpow(std::cos(om),e2),
                      b*sgnpow(std::cos(eta),e1)*sgnpow(std::sin(om),e2),
                      c*sgnpow(std::sin(eta),e1), 1)); } }
  return g;
}

static void computeSQ(){
  std::scoped_lock lock(sq_mutex);
  const double A=gui["sa"], B=gui["sb"], C=gui["sc"], E1=gui["se1"], E2=gui["se2"];
  const int num=gui["sqnum"], outPct=gui["sqout"]; const float noise=gui["sqnoise"];
  const int good=num*(100-outPct)/100;

  std::vector<Vec> pts; pts.reserve(num);
  utils::GRand gn(0,noise); utils::URand eta(-M_PI/2,M_PI/2), om(-M_PI,M_PI), obox(-5,5);
  for(int i=0;i<good;++i){ const double e=eta, o=om;
    pts.push_back(Vec(A*sgnpow(std::cos(e),E1)*sgnpow(std::cos(o),E2)+(float)gn,
                      B*sgnpow(std::cos(e),E1)*sgnpow(std::sin(o),E2)+(float)gn,
                      C*sgnpow(std::sin(e),E1)+(float)gn, 1)); }
  for(int i=good;i<num;++i) pts.push_back(Vec(obox,obox,obox,1));

  // CMA-ES fit of (a,b,c,e1,e2) minimising the Solina inside-outside error
  double ex=1e-3,ey=1e-3,ez=1e-3; for(const auto&p:pts){ ex=std::max(ex,(double)std::abs(p[0]));
    ey=std::max(ey,(double)std::abs(p[1])); ez=std::max(ez,(double)std::abs(p[2])); }
  auto cost=[&](const std::vector<double>&p)->double{
    const double a=std::abs(p[0])+1e-3,b=std::abs(p[1])+1e-3,c=std::abs(p[2])+1e-3;
    const double e1=std::min(2.0,std::max(0.1,p[3])), e2=std::min(2.0,std::max(0.1,p[4]));
    double s=0; for(const auto&q:pts){ const double f=sqInsideOut(q[0],q[1],q[2],a,b,c,e1,e2);
      const double r=std::sqrt(a*b*c)*(std::pow(f,e1/2)-1); s+=r*r; } return s;
  };
  const Time t0=Time::now();
  const auto res = sq_opt.minimize(cost, std::vector<double>{ex,ey,ez,1.0,1.0});
  const double ms=(Time::now()-t0).toMicroSecondsDouble()/1000.0;
  const double fa=std::abs(res.params[0]),fb=std::abs(res.params[1]),fc=std::abs(res.params[2]);
  const double fe1=std::min(2.0,std::max(0.1,res.params[3])), fe2=std::min(2.0,std::max(0.1,res.params[4]));

  PlotHandle3D plot = gui["plot3d"];
  plot->lock(); plot->clear();
  plot->setViewPort(Range32f(-4,4),Range32f(-4,4),Range32f(-4,4));
  plot->color(230,60,60,255); plot->nofill(); plot->pointsize(4); plot->scatter(pts);
  plot->nocolor(); plot->fill(40,120,255,90); plot->smoothfill(true);
  plot->surf(sqSurface(fa,fb,fc,fe1,fe2, 40,40), 40, 40);
  plot->unlock(); plot->render();

  std::ostringstream st; st<<std::fixed<<std::setprecision(2)
    <<"fit  size ("<<fa<<", "<<fb<<", "<<fc<<")  e ("<<fe1<<", "<<fe2<<")\n"
    <<"true size ("<<A<<", "<<B<<", "<<C<<")  e ("<<E1<<", "<<E2<<")   "<<std::setprecision(0)<<ms<<" ms";
  gui["sqstats"] = st.str();
}

// =====================================================================
void init(){
  std::ostringstream combo; for(size_t i=0;i<METHOD_NAMES.size();++i){ if(i) combo<<','; combo<<METHOD_NAMES[i]; }

  gui << (Tab("2D primitives,superquadric", {.handle="tabs"})
          << (HSplit()                                    // ---- tab 1 ----
              << Plot({.handle="plot2d", .minSize={32,24}})
              << (VBox().maxSize(17,99).minSize(15,1)
                  << Combo(combo.str(), {.handle="method"})
                  << (VBox({.handle="props2d", .label="parameters", .minSize={1,10}}))
                  << Slider(20,600,200,{.handle="num", .label="points"})
                  << FSlider(0.f,0.25f,0.04f,{.handle="noise", .label="noise"})
                  << Slider(0,80,25,{.handle="outliers", .label="outlier %"})
                  << Button("new data",{.handle="new2d"})
                  << Label("",{.handle="stats2d"})))
          << (HSplit()                                    // ---- tab 2 ----
              << Plot3D().handle("plot3d").minSize(32,24)
              << (VBox().maxSize(18,99).minSize(15,1)
                  << (VBox({.label="superquadric (ground truth)"})
                      << FSlider(0.5f,4.f,3.f,{.handle="sa", .label="a (x-size)"})
                      << FSlider(0.5f,4.f,2.f,{.handle="sb", .label="b (y-size)"})
                      << FSlider(0.5f,4.f,1.4f,{.handle="sc", .label="c (z-size)"})
                      << FSlider(0.1f,2.f,0.7f,{.handle="se1", .label="e1 (squareness)"})
                      << FSlider(0.1f,2.f,0.8f,{.handle="se2", .label="e2 (squareness)"}))
                  << (VBox({.handle="props3d", .label="CMA-ES"}))
                  << Slider(60,600,240,{.handle="sqnum", .label="points"})
                  << FSlider(0.f,0.3f,0.03f,{.handle="sqnoise", .label="noise"})
                  << Slider(0,60,10,{.handle="sqout", .label="outlier %"})
                  << Button("fit",{.handle="sqfit"})
                  << Label("",{.handle="sqstats"}))))
      << Show();

  gui["method"].registerCallback([]{ rebuildProps(gui["method"]); compute2D(); });
  gui["new2d"].registerCallback([]{ regenerate(); compute2D(); });
  gui["num"].registerCallback([]{ regenerate(); compute2D(); });
  gui["outliers"].registerCallback([]{ regenerate(); compute2D(); });
  gui["noise"].registerCallback([]{ regenerate(); compute2D(); });

  static GUI sqPropGUI;
  sqPropGUI = GUI(VBox());
  sqPropGUI << Prop(&sq_opt, {.label="CMA-ES"});
  sqPropGUI.create();
  gui.get<BoxHandle>("props3d").add(sqPropGUI.getRootWidget());
  for(const char *h : {"sa","sb","sc","se1","se2","sqnum","sqnoise","sqout","sqfit"})
    gui[h].registerCallback([]{ computeSQ(); });

  PlotHandle plot = gui["plot2d"];
  plot->prop("borders.left").value = 40;
  plot->prop("lock aspect ratio").value = true;

  rebuildProps(METHOD_NAMES[0]); compute2D();
  computeSQ();
}

void run(){
  if(gui["tabs"].as<int>() == 0) compute2D();   // 2D refits live; SQ refits on change
  Thread::msleep(60);
}

int main(int n, char **ppc){ return ICLApp(n, ppc, "", init, run).exec(); }
