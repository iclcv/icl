// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Model-fitting method playground: pick a fitting method from the combo, tune its
// parameters live via the auto-generated Prop panel, and watch the fit update on a
// scatter plot. Showcases the icl::math::fit framework — the same [a,b,c,d] Model
// flows through algebraic, Taubin, RANSAC/MSAC (RobustFitter) and algebraic→
// geometric (SeededFitter) methods interchangeably, and each method surfaces its
// own Configurable tunables. Analogous to icl-filter-playground.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/qt/BoxHandle.h>
#include <icl/math/fit/PrimitiveFitters2D.h>
#include <icl/math/fit/RobustFitter.h>
#include <icl/math/fit/RefiningFitter.h>
#include <icl/math/fit/GeometricRefiners2D.h>
#include <icl/utils/Random.h>
#include <icl/utils/Point.h>
#include <memory>
#include <mutex>
#include <iomanip>
#include <sstream>

using namespace icl::math;

using Model = std::vector<double>;
using MF    = ModelFitter<Point32f, Model>;
using RF    = RobustFitter<Point32f, Model>;
using RefF  = RefiningFitter<Point32f, Model>;

HSplit gui;

enum Shape { LINE, CIRCLE };

// The active fitting method: the top fitter we call fit() on and show Prop for,
// plus any owned sub-parts kept alive for the pipeline wrappers.
struct Method {
  std::unique_ptr<MF>   top;                 // fit() target + Prop root
  std::unique_ptr<MF>   sub;                 // base/seed for robust/seeded
  std::unique_ptr<RefF> refiner;             // for seeded
  RF                   *robust = nullptr;    // non-owning; set when top is robust
  Shape                 shape  = CIRCLE;
};

static const std::vector<std::string> METHOD_NAMES = {
  "circle: algebraic (Kasa)",
  "circle: Taubin",
  "circle: RANSAC/MSAC",
  "circle: Taubin + RANSAC",
  "circle: algebraic + geometric refine",
  "line: algebraic",
  "line: RANSAC/MSAC",
};

static Method makeMethod(const std::string &name){
  Method m;
  if(name == "circle: algebraic (Kasa)"){
    m.shape = CIRCLE; m.top.reset(new CircleFitter2D);
  }else if(name == "circle: Taubin"){
    m.shape = CIRCLE; m.top.reset(new TaubinCircleFitter);
  }else if(name == "circle: RANSAC/MSAC"){
    m.shape = CIRCLE; m.sub.reset(new CircleFitter2D);
    RF *r = new RF(m.sub.get(), 0.05, 0.99, 1000, "trimmed", true, 0.7); m.robust = r; m.top.reset(r);
  }else if(name == "circle: Taubin + RANSAC"){
    m.shape = CIRCLE; m.sub.reset(new TaubinCircleFitter);
    RF *r = new RF(m.sub.get(), 0.05, 0.99, 1000, "trimmed", true, 0.7); m.robust = r; m.top.reset(r);
  }else if(name == "circle: algebraic + geometric refine"){
    m.shape = CIRCLE; m.sub.reset(new CircleFitter2D);
    m.refiner.reset(new GeometricCircleRefiner);
    m.top.reset(new SeededFitter<Point32f, Model>(m.sub.get(), m.refiner.get()));
  }else if(name == "line: algebraic"){
    m.shape = LINE; m.top.reset(new LineFitter2D);
  }else if(name == "line: RANSAC/MSAC"){
    m.shape = LINE; m.sub.reset(new LineFitter2D);
    RF *r = new RF(m.sub.get(), 0.3, 0.99, 3000, "trimmed", true, 0.7); m.robust = r; m.top.reset(r);
  }
  return m;
}

// ---- shared state (compute() runs on the exec thread; callbacks on GUI) ----
static std::recursive_mutex g_mutex;
static Method               g_method;
static std::vector<Point32f> g_points;

// ground-truth shapes (small world coordinates so algebraic thresholds are intuitive)
static const float LINE_M = 0.6f, LINE_B = 0.4f;
static const float CX = 1.2f, CY = 0.5f, R = 1.5f;

static void regenerate(){
  std::scoped_lock lock(g_mutex);
  const int   num      = gui["num"].as<int>();
  const float noise    = gui["noise"].as<float>();
  const int   outPct   = gui["outliers"].as<int>();
  const int   good     = num * (100 - outPct) / 100;

  g_points.clear(); g_points.reserve(num);
  utils::GRand gn(0, noise);
  if(g_method.shape == CIRCLE){
    utils::URand ang(0, 2*M_PI), box(-1.5, 4.0);
    for(int i=0;i<good;++i){
      const double a = ang;
      g_points.push_back(Point32f(CX + R*std::cos(a) + (float)gn,
                                  CY + R*std::sin(a) + (float)gn));
    }
    for(int i=good;i<num;++i) g_points.push_back(Point32f(box, box));
  }else{ // LINE
    utils::URand x(-3, 3), oy(-3, 3);
    for(int i=0;i<good;++i){
      const float xx = x;
      g_points.push_back(Point32f(xx, LINE_M*xx + LINE_B + (float)gn));
    }
    for(int i=good;i<num;++i){ const float xx = x; g_points.push_back(Point32f(xx, oy)); }
  }
}

// geometric residual (for the RMS readout) from an [a,b,c(,d)] model
static double geomResidual(Shape s, const Model &m, const Point32f &p){
  if(s == CIRCLE){
    const double a=m[0], cx=-m[1]/(2*a), cy=-m[2]/(2*a);
    const double r=std::sqrt(std::max(0.0,(m[1]*m[1]+m[2]*m[2])/(4*a*a)-m[3]/a));
    return std::abs(std::hypot(p.x-cx, p.y-cy) - r);
  }
  return std::abs(m[0]*p.x + m[1]*p.y + m[2]) / std::hypot(m[0], m[1]);
}

static void drawModel(PlotHandle &plot, Shape s, const Model &m){
  if(!m.size()) return;
  if(s == CIRCLE){
    const double a=m[0];
    float c[3] = { float(-m[1]/(2*a)), float(-m[2]/(2*a)),
                   float(std::sqrt(std::max(0.0,(m[1]*m[1]+m[2]*m[2])/(4*a*a)-m[3]/a))) };
    plot->addAnnotations('c', c, 1, QColor(0,120,255));
  }else{
    auto y = [&](float x){ return (-m[2] - m[0]*x)/m[1]; };
    float seg[4] = { -3.f, (float)y(-3.f), 3.f, (float)y(3.f) };
    plot->addAnnotations('l', seg, 1, QColor(0,120,255));
  }
}

static void compute(){
  std::scoped_lock lock(g_mutex);
  if(!g_method.top || g_points.empty()) return;

  const Time t0 = Time::now();
  Model m = g_method.top->fit(g_points);
  const double ms = (Time::now() - t0).toMicroSecondsDouble() / 1000.0;

  PlotHandle plot = gui["plot"];
  plot->lock();
  plot->clear();
  plot->setDataViewPort(g_method.shape==CIRCLE ? Range32f(-1.6,4.1) : Range32f(-3.2,3.2),
                        g_method.shape==CIRCLE ? Range32f(-1.6,4.1) : Range32f(-3.2,3.2));

  std::vector<float> xs, ys; xs.reserve(g_points.size()); ys.reserve(g_points.size());
  for(const auto &p : g_points){ xs.push_back(p.x); ys.push_back(p.y); }
  plot->addScatterData('.', xs.data(), ys.data(), (int)xs.size(), "points", 230,60,60, 4);

  // inliers (robust methods) in green over the red
  int nin = -1;
  if(g_method.robust){
    const auto &in = g_method.robust->inliers();
    nin = (int)in.size();
    std::vector<float> ix, iy; ix.reserve(nin); iy.reserve(nin);
    for(const auto &p : in){ ix.push_back(p.x); iy.push_back(p.y); }
    if(nin) plot->addScatterData('.', ix.data(), iy.data(), nin, "inliers", 40,200,40, 4);
  }

  drawModel(plot, g_method.shape, m);
  plot->unlock();
  plot.render();

  // RMS geometric residual over the points used (inliers if robust)
  double sse = 0; int cnt = 0;
  auto acc = [&](const Point32f &p){ const double e = geomResidual(g_method.shape, m, p); sse += e*e; ++cnt; };
  if(g_method.robust && nin > 0) for(const auto &p : g_method.robust->inliers()) acc(p);
  else                           for(const auto &p : g_points) acc(p);
  const double rms = cnt ? std::sqrt(sse/cnt) : 0;

  std::ostringstream st;
  st << "RMS " << std::fixed << std::setprecision(4) << rms << " | ";
  if(nin >= 0) st << nin << "/" << g_points.size() << " inliers | ";
  st << std::setprecision(2) << ms << " ms";
  gui["stats"] = st.str();
}

static void rebuildProps(const std::string &name){
  static GUI propGUI;
  std::scoped_lock lock(g_mutex);

  BoxHandle box = gui.get<BoxHandle>("props");
  if(propGUI.hasBeenCreated()) propGUI.hide();   // detach old before destroying its fitter
  g_method = makeMethod(name);
  regenerate();

  propGUI = GUI(VBox());
  if(g_method.top) propGUI << Prop(g_method.top.get(), {.label=name});
  propGUI.create();
  box.add(propGUI.getRootWidget());
}

void init(){
  std::ostringstream combo;
  for(size_t i=0;i<METHOD_NAMES.size();++i){ if(i) combo << ','; combo << METHOD_NAMES[i]; }

  gui << (HSplit()
          << Plot({.handle="plot", .minSize={32,24}})
          << (VBox().maxSize(17,99).minSize(15,1)
              << Combo(combo.str(), {.handle="method"})
              << (VBox({.handle="props", .label="parameters", .minSize={1,10}}))
              << Slider(20, 600, 200, {.handle="num", .label="points"})
              << FSlider(0.f, 0.25f, 0.04f, {.handle="noise", .label="noise"})
              << Slider(0, 80, 25, {.handle="outliers", .label="outlier %"})
              << Button("new data", {.handle="new"})
              << Label("", {.handle="stats"})
              << ToggleButton("paused", "live", true, {.handle="live"})))
      << Show();

  gui["method"].registerCallback([]{ rebuildProps(gui["method"].as<std::string>()); compute(); });
  gui["new"].registerCallback([]{ regenerate(); compute(); });
  gui["num"].registerCallback([]{ regenerate(); compute(); });
  gui["outliers"].registerCallback([]{ regenerate(); compute(); });
  gui["noise"].registerCallback([]{ regenerate(); compute(); });

  PlotHandle plot = gui["plot"];
  plot->prop("borders.left").value = 40;
  plot->prop("lock aspect ratio").value = true;   // draw circles as circles

  rebuildProps(METHOD_NAMES[0]);
  compute();
}

void run(){
  if((bool)gui["live"]) compute();   // re-fit each tick so live param edits show
  Thread::msleep(60);
}

int main(int n, char **ppc){
  return ICLApp(n, ppc, "", init, run).exec();
}
