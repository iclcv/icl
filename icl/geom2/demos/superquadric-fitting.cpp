// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Superquadric fitting demo: sample noisy points on a superquadric surface (size +
// squareness set by sliders, plus noise/outliers), and fit the shape back with the
// icl::math::fit CMAESOptimizer (minimising the Solina inside-outside error). The
// point cloud + fitted surface render on a 3D Plot3D.
//
// The fit runs on the app's worker thread (run()), DETACHED from the GUI thread:
// slider callbacks only flag "inputs changed", so dragging stays responsive even
// while a fit is in flight; the worker always services the latest inputs.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/geom2/Plot3D.h>
#include <icl/math/fit/CMAESOptimizer.h>
#include <icl/utils/Random.h>
#include <atomic>
#include <mutex>
#include <iomanip>
#include <sstream>
#include <cmath>

using namespace icl::math;
using namespace icl::geom2;
using namespace icl::geom;

HSplit gui;

// ---- superquadric helpers ----
static inline double sgnpow(double b, double e){ return (b<0?-1.0:1.0)*std::pow(std::abs(b), e); }
static inline double sqInsideOut(double x,double y,double z, double a,double b,double c,double e1,double e2){
  return std::pow(std::pow(std::abs(x/a),2/e2)+std::pow(std::abs(y/b),2/e2), e2/e1)
       + std::pow(std::abs(z/c),2/e1);
}
static std::vector<Vec> sqSurface(double a,double b,double c,double e1,double e2,int nx,int ny){
  std::vector<Vec> g; g.reserve(nx*ny);
  for(int i=0;i<nx;++i){ const double eta=-M_PI/2 + M_PI*i/(nx-1);
    for(int j=0;j<ny;++j){ const double om=-M_PI + 2*M_PI*j/(ny-1);
      g.push_back(Vec(a*sgnpow(std::cos(eta),e1)*sgnpow(std::cos(om),e2),
                      b*sgnpow(std::cos(eta),e1)*sgnpow(std::sin(om),e2),
                      c*sgnpow(std::sin(eta),e1), 1)); } }
  return g;
}

// ---- shared input snapshot (GUI thread writes, worker reads) ----
struct Inputs { double a,b,c,e1,e2; int num,outPct; float noise; };
static std::mutex        in_mtx;
static Inputs            g_in;
static std::atomic<bool> g_dirty{true};
static CMAESOptimizer<std::vector<double>> sq_opt(1500, 0.4, 1e-12);

static void onInputsChanged(){                     // GUI thread — cheap, non-blocking
  std::scoped_lock lock(in_mtx);
  g_in = { gui["sa"], gui["sb"], gui["sc"], gui["se1"], gui["se2"],
           gui["num"].as<int>(), gui["outliers"].as<int>(), gui["noise"].as<float>() };
  g_dirty = true;
}

void init(){
  gui << (HSplit()
          << Plot3D().handle("plot").minSize(32,24)
          << (VBox().maxSize(18,99).minSize(15,1)
              << (VBox({.label="superquadric (ground truth)"})
                  << FSlider(0.5f,4.f,3.f,{.handle="sa", .label="a (x-size)"})
                  << FSlider(0.5f,4.f,2.f,{.handle="sb", .label="b (y-size)"})
                  << FSlider(0.5f,4.f,1.4f,{.handle="sc", .label="c (z-size)"})
                  << FSlider(0.1f,2.f,0.7f,{.handle="se1", .label="e1 (squareness)"})
                  << FSlider(0.1f,2.f,0.8f,{.handle="se2", .label="e2 (squareness)"}))
              << (VBox({.handle="props"}))
              << Slider(60,600,240,{.handle="num", .label="points"})
              << FSlider(0.f,0.3f,0.03f,{.handle="noise", .label="noise"})
              << Slider(0,60,0,{.handle="outliers", .label="outlier %"})
              << Label("",{.handle="stats"})))
      << Show();

  // fix the coord-box viewport once, on the GUI thread, before the worker fits —
  // otherwise the first render shows the dynamic (inf/nan) frame.
  { PlotHandle3D plot = gui["plot"];
    plot->setViewPort(Range32f(-4,4), Range32f(-4,4), Range32f(-4,4)); }

  static GUI propGUI(VBox().handle("propbox"));
  propGUI << Prop(&sq_opt, {.label="CMA-ES"});
  propGUI.create();
  gui.get<BoxHandle>("props").add(propGUI.getRootWidget());

  for(const char *h : {"sa","sb","sc","se1","se2","num","noise","outliers"})
    gui[h].registerCallback([]{ onInputsChanged(); });
  onInputsChanged();
}

void run(){                                        // worker thread — the heavy fit lives here
  if(!g_dirty){ Thread::msleep(30); return; }
  Inputs in; { std::scoped_lock lock(in_mtx); in = g_in; g_dirty = false; }

  // sample noisy surface points + outliers
  std::vector<Vec> pts; pts.reserve(in.num);
  const int good = in.num*(100-in.outPct)/100;
  utils::GRand gn(0,in.noise); utils::URand eta(-M_PI/2,M_PI/2), om(-M_PI,M_PI), obox(-5,5);
  for(int i=0;i<good;++i){ const double e=eta, o=om;
    pts.push_back(Vec(in.a*sgnpow(std::cos(e),in.e1)*sgnpow(std::cos(o),in.e2)+(float)gn,
                      in.b*sgnpow(std::cos(e),in.e1)*sgnpow(std::sin(o),in.e2)+(float)gn,
                      in.c*sgnpow(std::sin(e),in.e1)+(float)gn, 1)); }
  for(int i=good;i<in.num;++i) pts.push_back(Vec(obox,obox,obox,1));

  // CMA-ES fit of (a,b,c,e1,e2), centred axis-aligned, Solina inside-outside error
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

  PlotHandle3D plot = gui["plot"];
  plot->lock(); plot->clear();
  plot->color(230,60,60,255); plot->nofill(); plot->pointsize(4); plot->scatter(pts);
  plot->nocolor(); plot->fill(40,120,255,90); plot->smoothfill(true);
  plot->surf(sqSurface(fa,fb,fc,fe1,fe2, 40,40), 40, 40);
  plot->unlock(); plot->render();

  std::ostringstream st; st<<std::fixed<<std::setprecision(2)
    <<"fit  size ("<<fa<<", "<<fb<<", "<<fc<<")  e ("<<fe1<<", "<<fe2<<")\n"
    <<"true size ("<<in.a<<", "<<in.b<<", "<<in.c<<")  e ("<<in.e1<<", "<<in.e2<<")   "
    <<std::setprecision(0)<<ms<<" ms";
  gui["stats"] = st.str();
}

int main(int n, char **ppc){ return ICLApp(n, ppc, "", init, run).exec(); }
