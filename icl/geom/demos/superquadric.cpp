// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/Common.h>
#include <icl/geom/Geom.h>
#include <icl/utils/FPSLimiter.h>

// global data
HSplit gui;
Scene scene;
float sq[] = {0,0,0,0,0,0,1,1,1,1,1,30,30};
SceneObject *o = 0;
bool is_different(const float a[13],const float b[13]){
  for(unsigned int i=0;i<13;++i){
    if(fabs(a[i]-b[i]) > 0.0001){
      return true;
    }
  }
  return false;
}


void init(){
  gui << Canvas3D(Size::VGA).minSize(32,24).handle("draw")
      << ( VBox().minSize(15,1)
           << FSlider(-7,7,0).out("rx").label("x-rotation")
           << FSlider(-7,7,0).out("ry").label("y-rotation")
           << FSlider(-7,7,0).out("rz").label("z-rotation")

           << FSlider(0.1,10,1).out("dx").label("x-size")
           << FSlider(0.1,10,1).out("dy").label("y-size")
           << FSlider(0.1,10,1).out("dz").label("z-size")

           << ( HBox().label("e1")
                << CheckBox("1/x").out("e1x")
                << FSlider(1,10,1).out("e1")
                )
           << ( HBox().label("e1")
                << CheckBox("1/x").out("e2x")
                << FSlider(1,10,1).out("e2")
                )
           << Slider(5,100,30).out("step1").label("x-Steps")
           << Slider(5,100,30).out("step2").label("y-Steps")
           << CheckBox("grid").out("grid")
           )
      << Show();

  scene.addCamera(Camera(Vec(0,0,-10),Vec(0,0,1), Vec(1,0,0)));
  o = new SceneObject("superquadric",sq);
  o->setVisible(Primitive::vertex,false);
  o->setVisible(Primitive::line,false);
  scene.addObject(o,true);
  gui["draw"].install(scene.getMouseHandler(0));
  gui["draw"].link(scene.getGLCallback(0));

  SceneLight &l = scene.getLight(1);
  l.setOn();
  l.setDiffuseEnabled();
  l.setDiffuse(GeomColor(0,255,0,100));
  l.setPosition(Vec(10,10,10,1));
  l.setAnchorToWorld();

}


void run(){
  float sq_curr[] = { 0,0,0,
                      gui["rx"], gui["ry"], gui["rz"],
                      gui["dx"], gui["dy"], gui["dz"],
                      gui["e1"], gui["e2"],
                      gui["step1"], gui["step2"] };
  if(gui["e1x"]) sq_curr[9] = 1.0/sq_curr[9];
  if(gui["e2x"]) sq_curr[10] = 1.0/sq_curr[10];

  if (is_different(sq_curr, sq)){
    std::copy(sq_curr,sq_curr+13,sq);
    scene.lock();
    scene.removeObject(0);
    o = new SceneObject("superquadric",sq);
    scene.addObject(o,true);
    scene.unlock();
  }
  if(gui["grid"]){
    o->setVisible(Primitive::vertex,false);
    o->setVisible(Primitive::line,true);
    o->setVisible(Primitive::quad,false);
  }else{
    o->setVisible(Primitive::vertex,false);
    o->setVisible(Primitive::line,false);
    o->setVisible(Primitive::quad,true);
  }

  gui["draw"].render();
  static FPSLimiter limiter(25);
  limiter.wait();
}


int main(int n, char**ppc){
  return ICLApplication(n,ppc,"",init,run).exec();
}
