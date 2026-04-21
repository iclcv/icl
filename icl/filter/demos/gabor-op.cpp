// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/Common2.h>
#include <icl/utils/FPSEstimator.h>
#include <icl/filter/GaborOp.h>
HBox gui;
GenericGrabber grabber;

inline bool is_equal(const float *a, const float *b, unsigned int n){
  for(unsigned int i=0;i<n;i++){
    if(a[i] != b[i]) return false;
  }
  return true;
}
inline std::vector<float> vec1(float f) {
  return std::vector<float>(1,f);
}

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

void run(){
  float &lambda = gui.get<float>("lambda");
  float &theta = gui.get<float>("theta");
  float &psi = gui.get<float>("psi");
  float &gamma = gui.get<float>("gamma");
  float &sigma = gui.get<float>("sigma");
  int &width = gui.get<int>("width");
  int &height = gui.get<int>("height");

  float saveParams[] = {0,0,0,0,0};
  Size saveSize = Size::null;

  ImgBase *resultImage = 0;

  std::shared_ptr<GaborOp> g;

  while(1){
    float params[] = {lambda,theta,psi,gamma,sigma};
    Size size = Size(width,height);


    if(!is_equal(params,saveParams,5) || size != saveSize || !g){
      g.reset(new GaborOp(size,vec1(lambda),vec1(theta),vec1(psi),vec1(sigma),vec1(gamma)));
      Img32f m = g->getKernels()[0].detached();
      m.normalizeAllChannels(Range<float>(0,255));
      gui["mask"] = m;
    }
    saveSize = size;
    memcpy(saveParams,params,5*sizeof(float));

    g->apply(grabber.grabImage().ptr(),&resultImage);
    resultImage->normalizeAllChannels(Range<icl64f>(0,255));

    gui["image"] = resultImage;
    gui["fps"].render();
  }
}



int main(int n, char **ppc){
  return ICLApp(n,ppc,"[m]-input|-i(device,device-params) -format(format=rgb) -depth(depth=depth32f) -size(size=VGA)",init,run).exec();
}
