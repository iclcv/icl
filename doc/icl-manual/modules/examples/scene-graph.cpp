#include <ICLQt/Common.h>
#include <ICLGeom/Geom.h>

GUI gui;
Scene scene;

void init(){
  // create graphical user interface
  gui << Draw3D().handle("draw") << Show();

  // create camera and add to scene instance
  Camera cam(Vec(0,0,-10), // position
             Vec(0,0,1),   // view-direction
             Vec(1,0,0));  // up-direction
  scene.addCamera(cam);

  // add an object to the scene
  scene.addObject(SceneObject::cuboid(0,0,0,7,3,2));

  // use mouse events for camera movement
  gui["draw"].install(scene.getMouseHandler(0));

  // link the visualization
  gui["draw"].link(scene.getGLCallback(0));
}

int main(int n, char **args){
  return ICLApp(n,args,"",init).exec();
}
