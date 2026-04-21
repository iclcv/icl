// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/filter/AffineOp.h>
#include <icl/qt/Common2.h>
#include <icl/utils/StackTimer.h>

GUI gui = HSplit().minSize(32,24);
Img8u image;

void step(){
  AffineOp op(gui["interp"] ? interpolateNN : interpolateLIN);
  image.setROI(gui["clip"]?Rect(50,50,200,300):image.getImageRect());
  op.setClipToROI(gui["clip"]);

  op.scale(gui["scale"],gui["scale"]);
  op.rotate(gui["angle"].as<float>()*180/M_PI);

  gui["draw"] = op.apply(image);
  Thread::msleep(10);
}

void bench(){
  for(int i=0;i<100;++i){
    BENCHMARK_THIS_SECTION(linear interpolation);
    AffineOp op(interpolateLIN);
    image.setFullROI();
    op.scale(1.001,1.001);
    op.rotate(3.6*i);
    (void)op.apply(image);
  }
  for(int i=0;i<100;++i){
    BENCHMARK_THIS_SECTION(nn interpolation);
    AffineOp op(interpolateNN);
    image.setFullROI();
    op.scale(1.001,1.001);
    op.rotate(3.6*i);
    (void)op.apply(image);
  }
}

void init(){
  gui << Display().handle("draw").minSize(32,24)
      << ( VBox().maxSize(10,100)
           << FSlider(0.1,5,1,true).out("scale").label("scale").handle("a")
           << FSlider(0,6.3,0,true).out("angle").label("angle").handle("b")
           << Button("clip","off").label("clip ROI").out("clip").handle("c")
           << Button("lin","nn").label("interp.").out("interp").handle("d")
           << Button("bench").handle("bench")
          )
      << Show();

  image = scale(create("parrot"),0.4).as8u();

  gui.registerCallback(step,"a,b,c,d");
  gui.registerCallback(bench,"bench");
  gui["draw"] = image;
}


int main(int n, char **ppc){
  return ICLApplication(n,ppc,"",init).exec();
}
