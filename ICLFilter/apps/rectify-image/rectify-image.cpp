// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/Common.h>
#include <ICLQt/DefineQuadrangleMouseHandler.h>
#include <ICLFilter/ImageRectification.h>

HSplit gui;
GenericGrabber grabber;
DefineQuadrangleMouseHandler mouse;
ImgBase *rect = 0;
ImageRectification<icl8u> ir;

void init(){
  grabber.init(pa("-i"));
  grabber.useDesired(depth8u);

  gui << Canvas().handle("draw")
      << (VBox()
          << Display().handle("rectified")
          << (HBox().label("target size").maxSize(99,3)
              << Spinner(2,2000,512).handle("width")
              << Label("x")
              << Spinner(2,2000,512).handle("height")
             )
          << (HBox().label("rectify").maxSize(99,3)
              << Button("now").handle("now")
              << CheckBox("auto",true).handle("auto")
             )
         )
      << Show();

  mouse.init(grabber.grabImage().getSize());

  gui["draw"].install(&mouse);
}

void run(){
  DrawHandle draw = gui["draw"];
  ButtonHandle now = gui["now"];
  bool automatic = gui["auto"];

  const Img8u image = grabber.grabImage().as8u();
  draw = image;

  if(now.wasTriggered() || automatic){
    Size s(gui["width"],gui["height"]);
    std::vector<Point> ps = mouse.getQuadrangle();
    std::vector<Point32f> psf(ps.begin(),ps.end());
    try{
      const Img8u &rectf = ir.apply(psf.data(), image, s);
      gui["rectified"] = rectf;
    }catch(const ICLException &e){
      WARNING_LOG("rectification failed: " << e.what());
    }
  }

  draw->draw(mouse.vis());
  draw->render();
}

int main(int n, char **args){
  return ICLApp(n,args,"-input|-i(2)",init,run).exec();
}
