// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/Common.h>
#include <icl/core/Image.h>
#include <icl/utils/FPSLimiter.h>

GUI gui;
GenericGrabber grabber;

void run(){
  static FPSLimiter fps(pa("-maxfps"),10);

  gui["image"] = grabber.grabImage();
  gui["fps"].render();
  fps.wait();
}

void init(){
  gui << Display().handle("image").minSize(16,12);
  gui << ( HBox().maxSize(100,2)
           << Fps(10).handle("fps").maxSize(100,2).minSize(5,2)
           << CamCfg("")
           )
      << Show();

  grabber.init(pa("-i"));
  if(pa("-size")){
    grabber.useDesired<Size>(pa("-size"));
  }

}

int main(int n, char**ppc){
  pa_explain
  ("-input","define input grabber parameters\ne.g. -dc 0 or -file *.ppm\ngrabber parameters \"list 0\" will list all available grabbers")
  ("-size","desired image size of grabber");
  return ICLApp(n,ppc,"[m]-input|-i(device,device-params) "
		"-dist|-d(fn) "
                "-size|-s(Size) -maxfps(float=30) ",
                init,run).exec();
}
