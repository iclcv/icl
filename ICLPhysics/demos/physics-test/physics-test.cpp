#include <ICLGeom/Geom.h>
#include <ICLQt/Common.h>
#include <ICLUtils/FPSLimiter.h>
#include <ICLPhysics/PhysicsScene.h>
#include <ICLPhysics/RigidBoxObject.h>
#include <ICLPhysics/RigidSphereObject.h>
#include <ICLPhysics/PhysicsMouseHandler.h>
#include <ICLPhysics/SoftObject.h>

using namespace geom;
using namespace physics;
GUI gui;
FPSLimiter fps(60);
Camera cam(Vec(2000,0,700,1), Vec(-1,0,0,1), Vec(0,0,-1,1));
PhysicsScene scene;
PhysicsMouseHandler handler(0,&scene);
Vec start(0,0,100);
SoftObject *s;

void init(){
  scene.addCamera(cam);
  scene.addObject(new RigidBoxObject(0,0,0, 10000, 10000, 200,1),true);
  //scene.addObject(new RigidSphereObject(0,0,1000,100,1), true);
  s = new SoftObject("sphere.obj",&scene);

  gui << Draw3D().handle("draw") << Show();
  gui["draw"].install(&handler);
  gui["draw"].link(scene.getGLCallback(0));
}

int delay = 0;
void run()
{
  //delay the simulation for a second
  if(delay++ < fps.getMaxFPS()*2){
    scene.step(0.f);
  } else {
    scene.step();
  }
  gui["draw"].render();
  fps.wait();
}
int main(int n, char**ppc){
  return ICLApplication(n,ppc,"",init,run).exec();
}
