// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Andre Ueckermann, Christof Elbrechter

#include <icl/qt/Common2.h>
#include <icl/filter/MotionSensitiveTemporalSmoothing.h>
#include <icl/utils/Time.h>
#include <mutex>

VSplit gui;

std::unique_ptr<MotionSensitiveTemporalSmoothing> smoothing;
GenericGrabber grabber;

void init(){
  gui << Display().handle("image").minSize(32,24)
      << Display().handle("imageOut").minSize(32,24)
      << Slider(1,22,5).out("filterSize").label("filterSize").maxSize(100,2).handle("filterSize-handle")
      << Slider(1,22,10).out("difference").label("difference").maxSize(100,2).handle("difference-handle")
      << CheckBox("useCL", true).out("disableCL").maxSize(100,2).handle("disableCL-handle")
      << Show();

  int maxFilterSize=pa("-maxFilterSize");
  int nullValue=pa("-nullValue");

  smoothing = std::make_unique<MotionSensitiveTemporalSmoothing>(nullValue, maxFilterSize);
  grabber.init(pa("-i"));
}


void run(){
  static std::recursive_mutex mutex;
  std::scoped_lock<std::recursive_mutex> l(mutex);

  static ImageHandle image = gui["image"];
  static ImageHandle imageOut = gui["imageOut"];
  int filterSize = gui["filterSize"];
  int difference = gui["difference"];

  smoothing->setUseCL(gui["disableCL"].as<bool>());

  Image src = grabber.grabImage();
  Image dst;

  smoothing->setFilterSize(filterSize);
  smoothing->setDifference(difference);

  Time startT = Time::now();
  smoothing->apply(src, dst);
  Time endT = Time::now();
  std::cout << (smoothing->isCLActive() ? "OpenCL: " : "CPU: ");
  std::cout << (endT-startT).toMilliSeconds() << " ms" << std::endl;
  imageOut = dst;
  image = src;
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"[m]-input|-i(2) -nullValue|-nV(int=-1) -maxFilterSize|-mFS(int=20)",init,run).exec();
}
