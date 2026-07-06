// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/math/fit/RobustFitter.h>
#include <icl/utils/Random.h>
#include <icl/utils/Point.h>
#include <icl/math/fit/LeastSquareModelFitting2D.h>
#include <mutex>

typedef LeastSquareModelFitting2D LS;

/// ModelFitter adapter over LeastSquareModelFitting2D so RobustFitter can wrap it.
struct LSFitter : math::ModelFitter<Point32f, std::vector<double> > {
  LS *ls; int ns;
  LSFitter(LS *l, int n) : ls(l), ns(n) {}
  std::vector<double> fit(const std::vector<Point32f> &p) override { return ls->fit(p); }
  double residual(const std::vector<double> &m, const Point32f &p) const override { return ls->getError(m,p); }
  int minSamples() const override { return ns; }
};

HSplit gui;

Point32f get_line_point(const std::vector<double> &line, float x){
  return Point32f(x,(-line[2] - line[0]*x) / line[1]);
}

const std::vector<Point32f> gen_line_points(const std::vector<double> &line, bool noise, int num = 500){
  // ax + by + c = 0   ---> y(x) = (-c-ax)/b
  std::vector<Point32f> pts(num,Point32f(1,1));
  URand r(-1,1);
  GRand gr(0,gui["noise"].as<float>() * 2);

  int good = num* (noise ? gui["random"].as<float>()*0.01 : 1.0);
  for(int i=0;i<good;++i){
    pts[i] = get_line_point(line,r);
    if(noise) pts[i] += Point32f(0,gr);
  }
  for(int i=good;i<num;++i){
    pts[i] = Point32f((float)r,40.0*(float)r-20);
  }
  return pts;
}

const std::vector<Point32f> gen_circle_points(int num = 1000){
  std::vector<Point32f> pts(num,Point32f(1,1));
  URand r(-1,1);
  URand ar(0,2*M_PI);
  GRand gr(0,gui["noise"].as<float>() / 10);
  float cx = 3.5, cy = 2, radius = 1.5;
  int good = num * gui["random"].as<float>()*0.01;
  for(int i=0;i<good;++i){
    float a = ar;
    pts[i] = Point32f(cx + radius*::cos(a), cy + radius*sin(a)) + Point32f(gr,gr);
  }
  URand noiseX(1,8), noiseY(-2,5);
  for(int i=good;i<num;++i){
    pts[i] = Point32f(noiseX,noiseY);
  }
  return pts;
}

void compute(){
  static std::recursive_mutex mutex;
  std::scoped_lock lock(mutex);

  PlotHandle plot = gui["plot"];
  plot->lock();
  plot->clear();
  std::string what = gui["what"].as<std::string>();
  if(what == "line"){
    plot->setDataViewPort(Range32f(-1.1,1.1), Range32f(-60,20));
    plot->prop("tics.x-distance").value = 0.25;
    plot->prop("tics.y-distance").value = 10;


    const float line[] = {6,0.2,5}; // ax + by + c = 0
    const std::vector<double> LINE(line,line+3);

    std::vector<Point32f> ptsOrig = gen_line_points(LINE,true);

    plot->addScatterData('x',&ptsOrig[0].x,&ptsOrig[0].y, ptsOrig.size(), "input points",255,0,0, 3,false, 2,2);

    LeastSquareModelFitting2D ls(3,LeastSquareModelFitting2D::line_gen);

    if(gui["ransac"]){
      LSFitter base(&ls,3);
      math::RobustFitter<Point32f,std::vector<double> > fitLine(&base, 0.2, 0.99, 100, "ransac");
      std::vector<double> model = fitLine.fit(ptsOrig);
      const Point32f mps[2] = { get_line_point(model,-1), get_line_point(model,1) };
      plot->addAnnotations('l',&mps[0].x,1,QColor(0,100,255));
    }else{
      std::vector<double> model = ls.fit(ptsOrig);
      const Point32f mps[2] = { get_line_point(model,-1), get_line_point(model,1) };
      plot->addAnnotations('l',&mps[0].x,1,QColor(0,100,255));
    }
  }else if(what == "circle"){
    plot->setDataViewPort(Range32f(0,9),Range32f(-3,6));
    plot->prop("tics.x-distance").value = 1;
    plot->prop("tics.y-distance").value = 1;

    // a (x*x + y*y) + bx +cy +d = 0;
    std::vector<Point32f> ptsOrig = gen_circle_points();
    plot->addScatterData('x',&ptsOrig[0].x,&ptsOrig[0].y, ptsOrig.size(), "input points",255,0,0, 3,false, 2,2);

    LeastSquareModelFitting2D ls(4,LeastSquareModelFitting2D::circle_gen);

    std::vector<double> model;
    if(gui["ransac"]){
      LSFitter base(&ls,4);
      math::RobustFitter<Point32f,std::vector<double> > fitCircle(&base, 0.005, 0.99, 100, "ransac");
      model = fitCircle.fit(ptsOrig);
    }else{
      model = ls.fit(ptsOrig);
    }
    if(model.size()){
      float a = model[0], b = model[1], c = model[2], d = model [3];
      float cx = -b/(2*a), cy = -c/(2*a);
      float r = ::sqrt( (b*b+c*c)/(4*a*a) -d/a );
      float p[] = { cx,cy,r };
      plot->addAnnotations('c',p, 1, QColor(0,100,255));
    }
  }
  plot->unlock();
  plot.render();
}

void init(){
  gui << Plot({.handle="plot", .minSize={30,30}})
      << (VBox()
          << Combo("line,circle", {.handle="what"})
          << CheckBox("ransac", {.handle="ransac"})
          << Button("new data", {.handle="new"})
          << ToggleButton("stopped", "running", false, {.handle="run"})
          << ( HBox()
               << FSlider(0.01, 1, 0.2, {.vertical=true, .handle="noise", .label="noise", .tooltip="noise factor"})
               << Slider(0, 100, 30, {.vertical=true, .handle="random", .label="good %", .tooltip="percentage of non-random points"})
              )
          )
      << Show();


  gui["what"].registerCallback(compute);
  gui["new"].registerCallback(compute);
  gui["ransac"].registerCallback(compute);

  PlotHandle plot = gui["plot"];
  plot->prop("borders.left").value = 50;
  plot->setDataViewPort(Range32f(-1.1,1.1), Range32f(-60,20));

  compute();
}

void run(){
  while(!(bool)gui["run"]){
    Thread::msleep(10);
  }
  compute();
  Thread::msleep(20);
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"",init, run).exec();
}
