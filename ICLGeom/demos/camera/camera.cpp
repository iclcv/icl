// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter, Erik Weitnauer

#include <ICLGeom/Geom.h>
#include <ICLGeom/Scene.h>
#include <ICLGeom/Camera.h>
#include <ICLQt/Common.h>
#include <ICLUtils/FPSLimiter.h>
#include <ICLGeom/Primitive.h>

HSplit gui;
Scene scene;

void init(){

  gui << Canvas3D().minSize(16,12).handle("w1").label("Rendered into GL-Context")
      << Canvas3D().minSize(16,12).handle("w2").label("Rendered into GL-Context")
      << ( VBox().minSize(12,1)
           << FSlider(0.1,10,1.7).label("focal length left").out("fl")
           << FSlider(0.1,10,1.7).label("focal length right").out("fr")
           << FSlider(60,660,360).label("principal point offset x").out("px")
           << FSlider(40,440,240).label("principal point offset y").out("py")
           << FSlider(100,300,200).label("sampling resolution x").out("sx")
           << FSlider(100,300,200).label("sampling resolution y").out("sy")
           << FSlider(-100,100,0).label("skew").out("skew")
           << Fps(10).handle("fps")
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
