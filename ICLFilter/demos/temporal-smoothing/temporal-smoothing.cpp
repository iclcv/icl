// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Andre Ueckermann, Christof Elbrechter

#include <ICLQt/Common.h>
#include <ICLFilter/MotionSensitiveTemporalSmoothing.h>
#include <ICLFilter/ConvolutionOp.h>
#include <ICLUtils/Time.h>
#include <mutex>

VSplit gui;

MotionSensitiveTemporalSmoothing* smoothing;
GenericGrabber grabber;

void update();

void init(){
  gui << Display().handle("image").minSize(32,24)
      << Display().handle("imageOut").minSize(32,24)
      << Slider(1,22,5).out("filterSize").label("filterSize").maxSize(100,2).handle("filterSize-handle")
      << Slider(1,22,10).out("difference").label("difference").maxSize(100,2).handle("difference-handle")
      << CheckBox("useCL", true).out("disableCL").maxSize(100,2).handle("disableCL-handle")
      << Show();

  int maxFilterSize=pa("-maxFilterSize");
  int nullValue=pa("-nullValue");

  smoothing = new MotionSensitiveTemporalSmoothing(nullValue, maxFilterSize);
  grabber.init(pa("-i"));

  update();
}


void update(){
  static std::recursive_mutex mutex;
  std::lock_guard<std::recursive_mutex> l(mutex);

  static ImageHandle image = gui["image"];
  static ImageHandle imageOut = gui["imageOut"];
  int filterSize = gui["filterSize"];
  int difference = gui["difference"];

  if(gui["disableCL"]){
    smoothing->setUseCL(true);
  }else{
    smoothing->setUseCL(false);
  }

  static ImgBase *dst = 0;
  Image src = grabber.grabImage();

  smoothing->setFilterSize(filterSize);
  smoothing->setDifference(difference);

  Time startT, endT;
  startT = Time::now();
  smoothing->apply(src.ptr(), &dst);
  endT = Time::now();
  std::cout << (smoothing->isCLActive() ? "OpenCL: " : "CPU: ");
  std::cout << (endT-startT).toMilliSeconds() << " ms" << std::endl;
  imageOut = dst;
  image = src;
}

void run(){
  update();
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"[m]-input|-i(2) -nullValue|-nV(int=-1) -maxFilterSize|-mFS(int=20)",init,run).exec();
}
