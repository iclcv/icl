#include <icl/geom/Geom.h>
#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <algorithm>
#include <icl/physics/PhysicsScene2.h>
#include <icl/physics/RigidBoxObject.h>
#include <icl/physics/RigidSphereObject.h>
#include <icl/physics/PhysicsMouseHandler2.h>
#include <icl/physics/SixDOFConstraint.h>
#include "physics-maze-MazeObject.h"
#include "physics-maze-HoleObject.h"

using namespace geom;
using namespace physics;
GUI gui;
FPSLimiter fps(60);
Camera cam(Vec(200,0,70,1), Vec(-1,0,0,1), Vec(0,0,-1,1));
PhysicsScene2 scene;
PhysicsMouseHandler2 handler(0, &scene.getScene2(), &scene);
MazeObject* maze;

void init(){
  scene.addCamera(cam);
  scene.setBounds(300);  // calibrates camera dolly/pan speed to the scene
  maze = new MazeObject();
  maze->addToWorld(&scene);
  gui << ui::Canvas3D({.handle="draw"}) << ui::Show();
  gui["draw"].install(&handler);
  gui["draw"].link(scene.getGLCallback(0).get());
  scene.setGravity(Vec(0,0,-10000));
}

double fun(double val) {
  return std::clamp(val*5, -1., 1.);
}

utils::Time start = utils::Time::now();
void run()
{
  scene.lock();
  double time_val = (utils::Time::now() - start).toSecondsDouble();
  maze->setTransformation(Mat::id());
  maze->rotate(M_PI/180.f*fun(sin(time_val)),0,0);
  maze->rotate(0,M_PI/180.f*fun(cos(time_val)),0);
  Mat ball_trans = maze->mazeBall->getTransformation();
  Vec v(ball_trans(0, 3),ball_trans(1, 3),ball_trans(2, 3));
  std::cout<<maze->getWorldToMazeTransform()*v<<std::endl;
  scene.step(-1,50,1/120.);
  scene.unlock();
  scene.syncSceneFromPhysics();
  gui["draw"].render();
  fps.wait();
}

int main(int n, char**ppc){
  return ICLApplication(n,ppc,"",init,run).exec();
}
