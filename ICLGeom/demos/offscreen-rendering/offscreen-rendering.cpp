// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/Common.h>
#include <ICLGeom/Geom.h>
#include <ICLUtils/FPSLimiter.h>
#include <ICLUtils/Random.h>
#include <mutex>

// global data
HBox gui;
Scene scene;

struct OSRCube : public SceneObject{
  mutable std::recursive_mutex mutex;
  Img8u image;

  void lock() const override { mutex.lock(); }
  void unlock() const override { mutex.unlock(); }

  OSRCube():SceneObject("cube",Vec(0,0,0,3).data()){
    image = Img8u(Size(300,300),formatRGB);
    std::fill(image.begin(0),image.end(0),255);
    addTexture(0,1,2,3,&image,false);
    addTexture(7,6,5,4,&image,false);
    addTexture(0,3,7,4,&image,false);
    addTexture(5,6,2,1,&image,false);
    addTexture(4,5,1,0,&image,false);
    addTexture(3,2,6,7,&image,false);

    setVisible(Primitive::texture,true);
    setVisible(Primitive::quad,false);
  }

  int updateDisplay(){
    Time t = Time::now();
    const Img8u &screen = scene.render(0);
    gui["image"] = screen;


    std::lock_guard<std::recursive_mutex> lock(mutex);
    screen.deepCopy(&image);
    image.setROI(Rect(10,10,280,280));
    URand r(-50,50);
    static icl64f c[] = {128,128,128};
    for(int i=0;i<3;++i){
      c[i] += r;
      if(c[i]<0) c[i]=0;
      if(c[i]>255) c[i]=255;
    }

    image.fillBorder(std::vector<icl64f>(c,c+3));
    return (Time::now()-t).toMilliSeconds();
  }

} cube;

void init(){
  // create graphical user interface

  gui << Canvas3D(Size(300,300)).label("3D scene").minSize(16,16).handle("draw")
      << Canvas().minSize(16,16).handle("image").label("offscreen rendered image")
      << Show();

  // create camera and add to scene instance
  Camera cam(Vec(0,0,-13), // position
             Vec(0,0,1),   // view-direction
             Vec(1,0,0));   // up-direction
  cam.setResolution(Size(300,300));
  scene.addCamera(cam);

  scene.addObject(&cube);
  scene.setDrawCamerasEnabled(false);

  // link the scene's first camera with mouse gestures in the draw-GUI-component
  gui["draw"].install(scene.getMouseHandler(0));
}


void run(){
  DrawHandle3D draw = gui["draw"];
  DrawHandle image = gui["image"];

  int ms = cube.updateDisplay();
  image->color(255,255,255,255);
  image->text("offscreen rendering time: " + str(ms)+" ms",10,10,9);
  image.render();

  draw->link(scene.getGLCallback(0)); // render the whole scene
  draw.render();
}


int main(int n, char**ppc){
  /// create a whole application
  return ICLApplication(n,ppc,"",init,run).exec();
}
