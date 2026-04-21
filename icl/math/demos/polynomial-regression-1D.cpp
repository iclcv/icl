// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter
//
// 1D polynomial regression: fit y = f(x) to noisy data, visualize in 2D plot

#include <icl/qt/Common2.h>
#include <icl/math/PolynomialRegression.h>
#include <icl/utils/Random.h>

using Scalar = float;
using Matrix = DynMatrix<Scalar>;
using Reg = PolynomialRegression<Scalar>;

HSplit gui;

// Build polynomial function string: "1 + x0 + x0*x0 + ..."
std::string polyFunc(int degree) {
  std::string f = "1";
  for(int d = 1; d <= degree; ++d){
    f += " + ";
    for(int k = 0; k < d; ++k){
      if(k) f += "*";
      f += "x0";
    }
  }
  return f;
}

void init(){
  gui << Plot().handle("plot").minSize(50,35)
      << ( VBox().minSize(14,1)
           << Slider(1,7,3).handle("degree").label("polynomial degree")
           << Label("--").handle("fused").label("function")
           << Slider(10,500,100).handle("n").label("samples")
           << FSlider(0,2,0.3).handle("noise").label("noise")
           << Button("Refit").handle("refit")
         )
      << Show();
}

void run(){
  static ButtonHandle refit = gui["refit"];
  static int lastDegree = -1, lastN = -1;
  static float lastNoise = -1;

  int degree = gui["degree"].as<int>();
  int N = gui["n"].as<int>();
  float noiseLevel = gui["noise"];

  bool changed = (degree != lastDegree || N != lastN || noiseLevel != lastNoise);
  if(!refit.wasTriggered() && !changed) {
    Thread::msleep(100);
    return;
  }
  lastDegree = degree;
  lastN = N;
  lastNoise = noiseLevel;

  Reg reg(polyFunc(degree));
  gui["fused"] = reg.getFunctionString();

  URand rx(-3, 3);
  GRand noise(0, noiseLevel);

  Matrix xs(1, N);
  Matrix ys(1, N);
  std::vector<Point32f> dataPts(N);
  for(int i = 0; i < N; ++i){
    float x = rx;
    float y = 0.3f * sin(x * 2) + noise;
    xs[i] = x;
    ys[i] = y;
    dataPts[i] = Point32f(x, y);
  }

  const Reg::Result &result = reg.apply(xs, ys);

  constexpr int gridN = 200;
  Matrix gxs(1, gridN);
  for(int i = 0; i < gridN; ++i) gxs[i] = -3.0f + 6.0f * i / (gridN-1);
  const Matrix &gys = result(gxs);

  std::vector<Point32f> fitPts(gridN);
  for(int i = 0; i < gridN; ++i) fitPts[i] = Point32f(gxs[i], gys[i]);

  PlotHandle plot = gui["plot"];
  plot->clear();
  plot->setDataViewPort(Rect32f(-3.5, -2, 7, 4));
  plot->setPropertyValue("tics.x-distance", 1);
  plot->setPropertyValue("tics.y-distance", 0.5);

  plot->color(255, 80, 80);
  plot->sym('o');
  plot->scatter(dataPts, false);

  plot->color(40, 80, 255);
  plot->linewidth(2);
  plot->scatter(fitPts, true);
}

int main(int n, char **ppc){
  return ICLApp(n, ppc, "", init, run).exec();
}
