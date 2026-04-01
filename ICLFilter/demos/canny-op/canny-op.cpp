// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/Common.h>
#include <ICLCore/Image.h>
#include <ICLFilter/CannyOp.h>
#include <ICLFilter/ConvolutionOp.h>
#include <mutex>


VSplit gui;
GenericGrabber grabber;

void update();

void init(){
  gui << Display().handle("image").minSize(32,24)
      << (VBox()
          << FSlider(0,2000,10).out("low").label("low").maxSize(100,2).handle("low-handle")
          << FSlider(0,2000,100).out("high").label("high").maxSize(100,2).handle("high-handle")
          <<  ( HBox()
                << Slider(0,2,0).out("preGaussRadius").handle("pre-gauss-handle").label("pre gaussian radius")
                << Label("time").handle("dt").label("filter time in ms")
                << Button("stopped","running").out("running").label("capture")
                << CamCfg()
               )
          )
      << Show();

  grabber.init(pa("-i"));

  gui.registerCallback(update,"low-handle,high-handle,pre-gauss-handle");

  update();
}


void update(){
  static std::recursive_mutex mutex;
  std::lock_guard<std::recursive_mutex> l(mutex);

  static ImageHandle image = gui["image"];
  static LabelHandle dt = gui["dt"];
  float low = gui["low"];
  float high = gui["high"];
  int preGaussRadius = gui["preGaussRadius"];

  CannyOp canny(low,high,preGaussRadius);
  static Image dst;

  Time t = Time::now();
  canny.apply(grabber.grabImage(), dst);

  dt = (Time::now()-t).toMilliSecondsDouble();

  image = dst;
}

void run(){
  while(!gui["running"].as<bool>()){
    Thread::msleep(100);
  }
  Thread::msleep(1);
  update();
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"[m]-input|-i(2)",init,run).exec();


}
