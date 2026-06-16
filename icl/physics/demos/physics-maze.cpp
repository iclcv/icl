#include <icl/geom/Geom.h>
#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <algorithm>
#include <icl/physics/PhysicsScene2.h>
#include <icl/physics/RigidBoxObject.h>
#include <icl/physics/RigidSphereObject.h>
#include <icl/physics/SixDOFConstraint.h>
#include "physics-maze-MazeObject.h"
#include "physics-maze-HoleObject.h"

using namespace geom;
using namespace physics;
GUI gui;
FPSLimiter fps(60);
// maze lies in the xy-plane (~170x150), walls ~12 tall in +z — view from above
Camera cam(Vec(0,0,230,1), Vec(0,0,-1,1), Vec(0,1,0,1));
PhysicsScene2 scene;
MazeObject* maze;

// Maze tilt (radians), driven by left-drag. Gravity points into the screen
// (along the camera view axis), so tilting the maze rolls the ball.
float tiltX = 0, tiltY = 0;
utils::Point32f pressPos;
bool dragging = false;

void onMouse(const qt::MouseEvent &e){
  if(e.isPressEvent() && e.isLeft()){ pressPos = e.getRelPos(); dragging = true; }
  else if(e.isReleaseEvent()){ dragging = false; tiltX = tiltY = 0; }
  else if(e.isDragEvent() && dragging){
    utils::Point32f d = e.getRelPos() - pressPos;  // [-1,1]
    const float k = 1.5f, maxA = 0.6f;             // ~34 deg max tilt
    tiltY = std::clamp(d.x * k, -maxA, maxA);
    tiltX = std::clamp(-d.y * k, -maxA, maxA);
  }
}

void init(){
  scene.addCamera(cam);
  scene.setBounds(300);
  maze = new MazeObject();
  maze->addToWorld(&scene);
  gui << ui::Canvas3D({.handle="draw"}) << ui::Show();
  gui["draw"].install(&onMouse);             // left-drag tilts the maze
  gui["draw"].link(scene.getGLCallback(0).get());
  scene.setGravity(cam.getNorm() * 10000);   // along the camera view axis
}

void run()
{
  scene.lock();
  maze->setTransformation(Mat::id());
  maze->rotate(tiltX, 0, 0);
  maze->rotate(0, tiltY, 0);
  scene.step(-1,50,1/120.);
  scene.unlock();
  scene.syncSceneFromPhysics();
  gui["draw"].render();
  fps.wait();
}

int main(int n, char**ppc){
  return ICLApplication(n,ppc,"",init,run).exec();
}
