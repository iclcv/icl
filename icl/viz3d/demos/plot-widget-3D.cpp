// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// viz3d port of the legacy geom/plot-widget-3D demo, on the reimplemented
// viz3d Plot3D component: a scatter cloud + an animated function surface +
// a 3D label, inside an auto-scaling coordinate box.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/viz3d/Plot3D.h>
#include <icl/utils/Random.h>

using namespace icl::viz3d;
using namespace icl::geom;
using namespace icl::utils;
using namespace icl::math;
using namespace icl::qt;

using Vec3 = FixedColVector<float, 3>;

GUI gui;

static Range32f round_range(Range32f r) {
  if (r.minVal > r.maxVal) std::swap(r.minVal, r.maxVal);
  float m = fabs(r.maxVal - r.minVal), f = 1;
  if (m > 1) { while (m/f > 100) f *= 10; }
  else       { while (m/f < 10)  f *= 0.1; }
  r.minVal = floor(r.minVal/f) * f;
  r.maxVal = ceil(r.maxVal/f) * f;
  return r;
}

void init() {
  gui << Plot3D().handle("plot").minSize(32, 24) << Show();
  if (pa("-r")) {
    SHOW(round_range(Range32f(pa("-r",0), pa("-r",1))));
    ::exit(0);
  }
}

void add_scatter(PlotHandle3D &plot) {
  static const int N = 100000;
  static std::vector<Vec> ps(N);
  static std::vector<GeomColor> cs(N);

  GRand r(0, 4);
  FixedMatrix<float,3,3> T(0.000001,0,0, 0,0.000002,0, 0,0,0.000004);

  for (int i = 0; i < N; ++i) {
    Vec3 v = Vec3(1,1,1) + T * Vec3(r,r,r);
    ps[i] = Vec(v[0], v[1], v[2], 1);
    cs[i] = GeomColor(ps[i][0]-1, ps[i][1]-1, ps[i][2]-1, 0.0001);
  }
  plot->scatter(ps, cs, Range32f(-0.00001, 0.00001));
}

static Time tt = Time::now();
static float t = 0;

float f(float x, float y) {
  float r = sqrt(sqr(x-50) + sqr(y-50));
  return sin(0.2*r - 3*t) * exp(-(r*r)/800);
}

void add_function(PlotHandle3D &plot) {
  plot->setViewPort(Range32f(0,0), Range32f(0,0), Range32f(-3,3));
  plot->nocolor();
  t = tt.age().toSecondsDouble();
  static PlotWidget3D::Handle o = 0;
  o = plot->surf(f, Range32f(0,100), Range32f(0,100), 200, 200, o);
}

void add_label(PlotHandle3D &plot) {
  plot->color(255, 255, 255, 255);
  plot->label(Vec(50, 50, -3, 1), "sin(r)");
}

void run() {
  PlotHandle3D plot = gui["plot"];
  plot->lock();
  plot->clear();
  add_scatter(plot);
  add_function(plot);
  add_label(plot);
  plot->unlock();
  plot->render();
  Thread::msleep(30);
}

int main(int n, char **a) {
  return ICLApp(n, a, "-r(2)", init, run).exec();
}
