// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/Common2.h>
#include <icl/qt/DefineQuadrangleMouseHandler.h>
#include <icl/qt/ui.h>
#include <icl/filter/affine/ImageRectification.h>

HSplit gui;
ImageSource grabber;
DefineQuadrangleMouseHandler mouse;
ImageRectification<icl8u> ir;

void init(){
  grabber.init(pa("-i"));
  grabber.useDesired(depth8u);

  gui << ui::Canvas({.handle="draw"})
      << (ui::VBox()
          << ui::Display({.handle="rectified"})
          << (ui::HBox({.label="target size", .maxSize={99, 3}})
              << ui::Spinner(2, 2000, 512, {.handle="width"})
              << ui::Label("x")
              << ui::Spinner(2, 2000, 512, {.handle="height"})
             )
          << (ui::HBox({.label="rectify", .maxSize={99, 3}})
              << ui::Button("now", {.handle="now"})
              << ui::CheckBox("auto", {.checked=true, .handle="auto"})
             )
         )
      << ui::Show();

  mouse.init(grabber.grab().getSize());

  gui["draw"].install(&mouse);
}

void run(){
  DrawHandle draw = gui["draw"];
  ButtonHandle now = gui["now"];
  bool automatic = gui["auto"];

  const Img8u image = grabber.grab().as8u();
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
