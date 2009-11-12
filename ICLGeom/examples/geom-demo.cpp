#include <iclCommon.h>
#include <iclGeom.h>
#include <iclFPSLimiter.h>

GUI gui;
Scene2 scene;

void init(){
  // create graphical user interface
  gui << "draw3D[@minsize=16x12@handle=draw@label=scene view]"
      << "fslider(0.5,20,3)[@out=f@handle=focal"
         "@label=focal length@maxsize=100x3]";
  gui.show();
  
  // create scene background
  gui["draw"] = zeros(640,480,0);

  // create camera and add to scene instance
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
  scene.getCamera(0).setFocalLength(gui.getValue<float>("f"));

  draw->lock();
  draw->reset3D();
  draw->callback(scene.getGLCallback(0));
  draw->unlock();
  draw.update();

  limiter.wait();
}


int main(int n, char**ppc){
  return ICLApplication(n,ppc,"",init,run).exec();
}

