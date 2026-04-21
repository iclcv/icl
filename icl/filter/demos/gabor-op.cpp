// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/Common2.h>
#include <icl/filter/GaborOp.h>
HBox gui;
GenericGrabber grabber;

void init(){
  gui << Display().minSize(32,24).label("Result Image").handle("image")
      << (VBox().handle("sidebar")
          << ( HBox()
               << Display().minSize(15,15).label("Gabor Mask").handle("mask")
               << Fps(10).handle("fps")
              )
          << (VBox()
              << FSlider(0.1,100,20).label("Wave-Length -Lambda-").minSize(15,2).out("lambda")
              << FSlider(0,3.15,0).label("Wave-Angle -Theta-").minSize(15,2).out("theta")
              << FSlider(0,50,0).label("Phase-Offset -Psi-").minSize(15,2).out("psi")
              << FSlider(0.01,10,0.5).label("Elipticity -Gamma-").minSize(15,2).out("gamma")
              << FSlider(0.1,18,20).label("Gaussian Std-Dev. -Sigma-").minSize(15,2).out("sigma")
              << Slider(3,50,10).label("Width").minSize(15,2).out("width")
              << Slider(3,50,10).label("Height").minSize(15,2).out("height")
             )
          )
      << Show();

  grabber.init(pa("-i"));
  grabber.useDesired(parse<Size>(pa("-size")));
  grabber.useDesired(parse<format>(*pa("-format")));
  grabber.useDesired(parse<depth>(*pa("-depth")));
}

inline std::vector<float> vec1(float f) {
  return std::vector<float>(1,f);
}

void run(){
  float lambda = gui["lambda"];
  float theta = gui["theta"];
  float psi = gui["psi"];
  float gamma = gui["gamma"];
  float sigma = gui["sigma"];
  int width = gui["width"];
  int height = gui["height"];

  static float saveParams[] = {0,0,0,0,0};
  static Size saveSize = Size::null;
  static std::shared_ptr<GaborOp> g;

  float params[] = {lambda,theta,psi,gamma,sigma};
  Size size(width,height);

  if(!std::equal(params, params+5, saveParams) || size != saveSize || !g){
    g = std::make_shared<GaborOp>(size,vec1(lambda),vec1(theta),vec1(psi),vec1(sigma),vec1(gamma));
    Img32f m = g->getKernels()[0].detached();
    m.normalizeAllChannels(Range<float>(0,255));
    gui["mask"] = m;
  }
  saveSize = size;
  std::copy(params, params+5, saveParams);

  Image result = g->apply(grabber.grabImage());
  result.ptr()->normalizeAllChannels(Range<icl64f>(0,255));

  gui["image"] = result;
  gui["fps"].render();
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"[m]-input|-i(device,device-params) -format(format=rgb) -depth(depth=depth32f) -size(size=VGA)",init,run).exec();
}
