// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/filter/AffineOp.h>
#include <icl/qt/Common2.h>
#include <icl/utils/StackTimer.h>

GUI gui = HSplit().minSize(32,24);
Image baseImage;   // pristine source, untouched
Image image;       // working copy: baseImage with optional ROI overlay

const Rect ROI_RECT(50, 50, 200, 300);

void updateSource(){
  image = baseImage.deepCopy();

  // Red border around the full image, always drawn — makes the actual
  // image boundary visible regardless of the ROI toggle.
  color(255, 0, 0, 255);
  fill(0, 0, 0, 0);
  rect(image, image.getImageRect());

  if(gui["hasROI"]){
    color(0, 255, 0, 255);   // solid green border
    fill(0, 255, 0, 51);     // 20% alpha green fill (0.2 * 255 ≈ 51)
    rect(image, ROI_RECT);
    image.setROI(ROI_RECT);
  }else{
    image.setROI(image.getImageRect());
  }
}

void step(){
  static AffineOp nn(interpolateNN), lin(interpolateLIN);
  nn.reset();
  lin.reset();

  AffineOp &op = gui["interp"] ? nn : lin;
  op.setClipToROI(gui["clipToROI"]);
  op.scale(gui["scale"], gui["scale"]);
  op.rotate(gui["angle"].as<float>() * 180 / M_PI);

  gui["draw"] = op.apply(image);
  static utils::FPSLimiter limiter(30);
  limiter.wait();
}

void bench(){
  Image benchImage = scale(create("parrot"),1000, 1000).as8u();
  AffineOp lin(interpolateLIN);
 
  Time t = Time::now();
  for(int i=0;i<100;++i){   
    lin.reset();    
    lin.scale(1.001,1.001);
    lin.rotate(3.6*i);   
    (void)lin.apply(benchImage);
  }
  t.showAge("100 affine ops with linear interpolation");
  AffineOp nn(interpolateNN);
  t = Time::now();
  for(int i=0;i<100;++i){
    nn.reset();    
    nn.scale(1.001,1.001);
    nn.rotate(3.6*i);    
    (void)nn.apply(benchImage);
  }
  t.showAge("100 affine ops with nearest neighbor interpolation");
}

void init(){
  gui << Display().handle("draw").minSize(32,24)
      << ( VBox().minSize(20,1)
           << FSlider(0.1,5,1).out("scale").label("scale").handle("scaleH")
           << FSlider(0,6.3,0).out("angle").label("angle").handle("angleH")
           << CheckBox("source image has ROI", false).out("hasROI").handle("hasROIH")
           << CheckBox("clipToROI", false).out("clipToROI").handle("clipH")
           << Button("lin","nn").label("interp.").out("interp").handle("interpH")
           << Button("bench").handle("bench")
          )
      << Show();

  baseImage = scale(create("parrot"), 0.4).as8u();
  updateSource();
  // Regenerate the source image only when the ROI toggle flips.
  gui.registerCallback(updateSource, "hasROIH");
  // Any control change re-runs the transform.
  gui.registerCallback(step, "scaleH,angleH,hasROIH,clipH,interpH");
  gui.registerCallback(bench, "bench");
  gui["draw"] = image;
}


int main(int n, char **ppc){
  return ICLApplication(n,ppc,"",init).exec();
}
