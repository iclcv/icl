/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/examples/model-fitting/model-fitting.cpp         **
** Module : ICLQt                                                  **
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
#include <ICLMath/RansacFitter.h>
#include <ICLUtils/Random.h>
#include <ICLUtils/Point32f.h>
#include <ICLMath/LeastSquareModelFitting2D.h>

typedef LeastSquareModelFitting2D LS;
typedef RansacFitter<Point32f,std::vector<double> > RANSAC;

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
  static Mutex mutex;
  Mutex::Locker lock(mutex);

  PlotHandle plot = gui["plot"];
  plot->lock();
  plot->clear();
  std::string what = gui["what"].as<std::string>();
  if(what == "line"){
    plot->setDataViewPort(Range32f(-1.1,1.1), Range32f(-60,20));
    plot->setPropertyValue("tics.x-distance",0.25);
    plot->setPropertyValue("tics.y-distance",10);

  
    const float line[] = {6,0.2,5}; // ax + by + c = 0
    const std::vector<double> LINE(line,line+3);

    std::vector<Point32f> ptsOrig = gen_line_points(LINE,true);
    
    plot->addScatterData('x',&ptsOrig[0].x,&ptsOrig[0].y, ptsOrig.size(), "input points",255,0,0, 3,false, 2,2);
    
    LeastSquareModelFitting2D ls(3,LeastSquareModelFitting2D::line_gen); 

    if(gui["ransac"]){
      RANSAC::ModelFitting fit = function(ls,&LS::fit);
      RANSAC::PointError err = function(ls,&LS::getError);
      RANSAC fitLine(5,100,fit,err,0.2,30);
      RANSAC::Result r = fitLine.fit(ptsOrig);
      const Point32f mps[2] = { get_line_point(r.model,-1), get_line_point(r.model,1) };
      plot->addAnnotations('l',&mps[0].x,1,QColor(0,100,255));
    }else{
      std::vector<double> model = ls.fit(ptsOrig);
      const Point32f mps[2] = { get_line_point(model,-1), get_line_point(model,1) };
      plot->addAnnotations('l',&mps[0].x,1,QColor(0,100,255));
    }
  }else if(what == "circle"){
    plot->setDataViewPort(Range32f(0,9),Range32f(-3,6));
    plot->setPropertyValue("tics.x-distance",1);
    plot->setPropertyValue("tics.y-distance",1);

    // a (x*x + y*y) + bx +cy +d = 0;
    std::vector<Point32f> ptsOrig = gen_circle_points();
    plot->addScatterData('x',&ptsOrig[0].x,&ptsOrig[0].y, ptsOrig.size(), "input points",255,0,0, 3,false, 2,2);

    LeastSquareModelFitting2D ls(4,LeastSquareModelFitting2D::circle_gen); 
    
    std::vector<double> model;
    if(gui["ransac"]){
      RANSAC::ModelFitting fit = function(ls,&LS::fit);
      RANSAC::PointError err = function(ls,&LS::getError);
      RANSAC fitCircle(5,100,fit,err,0.005,40);
      RANSAC::Result result = fitCircle.fit(ptsOrig);
      model = result.model;
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
  gui << Plot().handle("plot").minSize(30,30)
      << (VBox()
          << Combo("line,circle").handle("what")
          << CheckBox("ransac").handle("ransac")
          << Button("new data").handle("new")
          << Button("stopped","running").handle("run")
          << ( HBox()
               << FSlider(0.01,1,0.2,true).out("noise").label("noise").tooltip("noise factor")
               << Slider(0,100,30,true).out("random").label("good %").tooltip("percentage of non-random points")
              )
          )
      << Show();

  
  gui["what"].registerCallback(compute);
  gui["new"].registerCallback(compute);
  gui["ransac"].registerCallback(compute);

  PlotHandle plot = gui["plot"];
  plot->setPropertyValue("borders.left",50);
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
