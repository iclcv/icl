// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/Common2.h>
#include <icl/core/Image.h>
#include <icl/filter/CannyOp.h>
#include <icl/filter/ConvolutionOp.h>
#include <mutex>


VSplit gui;
GenericGrabber grabber;
CannyOp canny;

void init(){
  gui << Display().handle("image").minSize(32,24)
      << (VBox()
          << FSlider(0,2000,10).out("low").label("low").maxSize(100,2).handle("low-handle")
          << FSlider(0,2000,100).out("high").label("high").maxSize(100,2).handle("high-handle")
          <<  ( HBox()
                << Slider(0,2,0).out("preGaussRadius").handle("pre-gauss-handle").label("pre gaussian radius")
                << Label("time").handle("dt").label("filter time in ms")
                << CamCfg()
               )
          )
      << Show();

  grabber.init(pa("-i"));
}


void run(){
  canny.setThresholds(gui["low"], gui["high"]);
  canny.setPreBlurRadius(gui["preGaussRadius"]);

  Time t = Time::now();
  Image result = canny.apply(grabber.grabImage());
  gui["dt"] = (Time::now()-t).toMilliSecondsDouble();

  gui["image"] = result;

  static FPSLimiter limiter(30);
  limiter.wait();
}



int main(int n, char **ppc){
  return ICLApplication(n,ppc,"[m]-input|-i(2)",init,run).exec();


}
