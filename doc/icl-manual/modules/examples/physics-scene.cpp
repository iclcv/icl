#include <ICLGeom/Geom.h>
#include <ICLQt/Common.h>
#include <ICLUtils/FPSLimiter.h>

#include <ICLPhysics/PhysicsScene.h>
#include <ICLPhysics/RigidBoxObject.h>
#include <ICLPhysics/RigidCylinderObject.h>
#include <ICLPhysics/RigidSphereObject.h>
#include <ICLPhysics/PhysicsMouseHandler.h>

using namespace geom;
using namespace physics;

GUI gui;
FPSLimiter fps(60);
Camera cam(Vec(2000,0,500,1), Vec(-1,0,0,1), Vec(0,0,-1,1));

//defining the position, dimensions and weight of the objects in the scene
RigidBoxObject box(0,0,500, 100, 100, 100, 0.1);
RigidCylinderObject cylinder(0,-20,700.0, 100, 100 , 0.1);
RigidSphereObject sphere(-200,0,90,100,0.1);
//a weight of zero makes objects static
RigidBoxObject table(0,0,-200.0, 10000, 10000, 200, 0);
PhysicsScene scene;

PhysicsMouseHandler handler(0,&scene);

void init(){
  gui << Draw3D().handle("draw") << Show();
  scene.addCamera(cam);

  table.setColor(Primitive::all,geom_red());

  //restition defines how bouncy an object is
  cylinder.setRestitution(0.5f);
  sphere.setRestitution(0.5f);
  box.setRestitution(0.5f);
  table.setRestitution(0.9f);

  //friction describes the damping behaviour when two objects slide against each other
  box.setFriction(0.5f);
  cylinder.setFriction(0.5f);
  sphere.setFriction(0.5f);
  table.setFriction(0.5f);

  //rolling frction describes the angular damping behaviour when one object rolls over another
  cylinder.setRollingFriction(0.2f);
  sphere.setRollingFriction(0.1f);
  table.setRollingFriction(0.2f);
  box.setRollingFriction(0.1f);

  scene.addObject(&cylinder);
  scene.addObject(&sphere);
  scene.addObject(&box);
  scene.addObject(&table);

  gui["draw"].install(&handler);
  
  //link the visualization
  gui["draw"].link(scene.getGLCallback(0));
}

void run()
{
  scene.step();
  gui["draw"].render();
  fps.wait();
}

int main(int n, char**ppc){
  return ICLApplication(n,ppc,"",init,run).exec();
}
