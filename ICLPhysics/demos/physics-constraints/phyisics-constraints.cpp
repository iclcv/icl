#include <ICLGeom/Geom.h>
#include <ICLQt/Common.h>
#include <ICLUtils/FPSLimiter.h>

#include <ICLPhysics/PhysicsScene.h>
#include <ICLPhysics/RigidBoxObject.h>
#include <ICLPhysics/RigidCylinderObject.h>
#include <ICLPhysics/RigidSphereObject.h>
#include <ICLPhysics/HingeConstraint.h>
#include <ICLPhysics/PhysicsMouseHandler.h>
using namespace geom;
using namespace physics;
using namespace std;

GUI gui;
FPSLimiter fps(60);
Camera cam(Vec(2000,0,500,1), Vec(-1,0,0,1), Vec(0,0,-1,1));


PhysicsScene scene;

PhysicsMouseHandler handler(0,&scene);

void init(){
  gui << Draw3D().handle("draw") << Show();
  scene.addCamera(cam);

  RigidBoxObject *door = new RigidBoxObject(0,115,200, 20, 200, 390, 0.1);
  RigidCylinderObject *hinge = new RigidCylinderObject(0,0,200,20,400, 0.0);
  RigidBoxObject *ground = new RigidBoxObject(0.0,0.0,-10, 1000, 1000, 20, 0);
  HingeConstraint *hinge_constraint = new HingeConstraint(hinge, door, Vec(0,0,0), Vec(0,-115,0), 2);

  ground->setColor(Primitive::all,geom_red());

  //simulate friction between the hinge and the door by adding some damping
  door->setDamping(0.2,0.2);

  scene.addObject(door,true);
  scene.addObject(hinge,true);
  scene.addObject(ground,true);
  scene.addConstraint(hinge_constraint,false,true);

  gui["draw"].install(&handler);
  
  //link the visualization
  gui["draw"].link(scene.getGLCallback(0));
}

int delay = 0;
void run()
{
  //delay the simulation for a second
  if(delay++ < fps.getMaxFPS()){
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
