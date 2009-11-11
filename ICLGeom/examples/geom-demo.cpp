#include <iclQuick.h>
#include <iclGeom.h>

#include <iclScene2.h>
#include <iclCamera.h>
#include <iclCommon.h>
#include <iclFPSLimiter.h>
#include <iclPrimitive.h>

GUI gui;

Scene2 scene;

void init(){
  gui << "draw3D[@minsize=16x12@handle=draw@label=scene view]"
      << "fslider(0.5,20,3)[@out=f@handle=focal"
         "@label=focal length@maxsize=100x3]";
  gui.show();
  
  // create scene background
  gui["draw"] = zeros(640,480,0);

  // create camera
  Camera cam(Vec(0,0,-10), // position
             Vec(0,0,1),   // view-direction
             Vec(1,0,0),   // up-direction
             Size::VGA);   // screen-size
  scene.addCamera(cam);

  // add an object to the scene
  float data[] = {0,0,0,7,3,2};
  scene.addObject(new Object2("cuboid",data));

  // use mouse events for camera movement
  gui["draw"].install(scene.getMouseHandler(0));
}


void run(){
  /// limit drawing speed to 25 fps
  static FPSLimiter limiter(25);
  gui_DrawHandle3D(draw);
  gui_float(f);
  scene.getCamera(0).setFocalLength(f);

  draw->lock();
  draw->reset3D();
  draw->callback(scene.getGLCallback(0));
  draw->unlock();
  limiter.wait();
  
  draw.update();
}


int main(int n, char**ppc){
  return ICLApplication(n,ppc,"",init,run).exec();
}

