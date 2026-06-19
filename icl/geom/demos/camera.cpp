// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Erik Weitnauer

#include <icl/geom/Geom.h>
#include <icl/geom/Scene.h>
#include <icl/geom/Camera.h>
#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/geom/Primitive.h>

HSplit gui;
Scene scene;

void init(){

  gui << Canvas3D({.handle="w1", .label="Rendered into GL-Context", .minSize={16, 12}})
      << Canvas3D({.handle="w2", .label="Rendered into GL-Context", .minSize={16, 12}})
      << ( VBox({.minSize={12, 1}})
           << FSlider(0.1, 10, 1.7, {.handle="fl", .label="focal length left"})
           << FSlider(0.1, 10, 1.7, {.handle="fr", .label="focal length right"})
           << FSlider(60, 660, 360, {.handle="px", .label="principal point offset x"})
           << FSlider(40, 440, 240, {.handle="py", .label="principal point offset y"})
           << FSlider(100, 300, 200, {.handle="sx", .label="sampling resolution x"})
           << FSlider(100, 300, 200, {.handle="sy", .label="sampling resolution y"})
           << FSlider(-100, 100, 0, {.handle="skew", .label="skew"})
           << Fps(10, {.handle="fps"})
           )
      << Show();

  scene.addCamera(Camera(Vec(-250,0,1000,1),Vec(0,0,-1,1),Vec(0,1,0,1)));
  scene.addCamera(Camera(Vec(200,0,200,1),Vec(-1,0,0,1),Vec(0,1,0,1)));
  scene.setDrawCoordinateFrameEnabled(true,120);

  gui["w1"].install(scene.getMouseHandler(0));
  gui["w2"].install(scene.getMouseHandler(1));

  scene.getCamera(0).setName("Left Camera");
  scene.getCamera(1).setName("Right Camera");

  gui["w1"].link(scene.getGLCallback(0));
  gui["w2"].link(scene.getGLCallback(1));
}


void run(){
  Camera &l = scene.getCamera(0), &r = scene.getCamera(1);
  l.setFocalLength(gui["fl"]);
  r.setFocalLength(gui["fr"]);

  l.setPrincipalPointOffset(Point32f(gui["px"],gui["py"]));
  l.setSamplingResolution(gui["sx"],gui["sy"]);

  l.setSkew(gui["skew"]);


  gui["fps"].render();
  gui["w1"].render();
  gui["w2"].render();

  static FPSLimiter limiter(25);
  limiter.wait();
}


int main(int n, char**ppc){
  ERROR_LOG("this demo has a bug!");
  return ICLApplication(n,ppc,"",init,run).exec();
}
