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
Camera cam(Vec(800,0,50), Vec(-1,0.3,0), Vec(0,0,-1));


PhysicsScene scene;

PhysicsMouseHandler handler(0,&scene);

void init(){
  gui << Draw3D().handle("draw") << Show();
  scene.addCamera(cam);
  RigidBoxObject *ground = new RigidBoxObject(0,0,-200, 5000, 5000, 200, 0);
  Vec car_size(100,200,70);
  int wheel_radius = 50;
  RigidBoxObject *chasis = new RigidBoxObject(0,0,car_size[2]/2, car_size[0], car_size[1], car_size[2], 1);

  //add the wheels to the scene
  RigidCylinderObject* wheels[4];
  for(int i = 0; i < 4; i++) {
    wheels[i] = new RigidCylinderObject(0,0,-100,wheel_radius,10,0.1);
    wheels[i]->setFriction(1.f);
    wheels[i]->setRestitution(0.f);
    wheels[i]->rotate(0.1,0.1,0.1);
    wheels[i]->setDamping(0.1f,0.2f);
    wheels[i]->setRollingFriction(0.1f);
    scene.addObject(wheels[i],true);
  }

  //add the bricks to the scene
  for(int x = 0; x < 5; x++) {
    for(int z = 0; z < 10; z++) {
      RigidBoxObject *brick = new RigidBoxObject(x*100-450+z%2*20,500,z*40-80, 100, 100, 40, 0.1);
      brick->setFriction(0.5f);
      brick->setDamping(0.1,0.1);
      brick->setRestitution(0.f);
      scene.addObject(brick,true);
    }
  }

  HingeConstraint* wheel_cons[4];

  Mat frameB(0,0,-1,0,
             0,1,0,0,
             -1,0,0,0,
             0,0,0,1);
  //front left
  Mat frameA(1,0,0,-car_size[0]/2,
             0,1,0,(car_size[1]- wheel_radius)/2,
             0,0,1,-car_size[2]/2,
             0,0,0,1);
  wheel_cons[0] = new HingeConstraint(chasis,wheels[0],frameA,frameB,0);
  //front right
  frameA = Mat(1,0,0,car_size[0]/2,
               0,1,0,(car_size[1]- wheel_radius)/2,
               0,0,1,-car_size[2]/2,
               0,0,0,1);
  wheel_cons[1] = new HingeConstraint(chasis,wheels[1],frameA,frameB,0);
  //back left
  frameA = Mat(1,0,0,-car_size[0]/2,
               0,1,0,-(car_size[1]- wheel_radius)/2,
               0,0,1,-car_size[2]/2,
               0,0,0,1);
  wheel_cons[2] = new HingeConstraint(chasis,wheels[2],frameA,frameB,0);
  //back right
  frameA = Mat(1,0,0,car_size[0]/2,
               0,1,0,-(car_size[1]- wheel_radius)/2,
               0,0,1,-car_size[2]/2,
               0,0,0,1);
  wheel_cons[3] = new HingeConstraint(chasis,wheels[3],frameA,frameB,0);

  ground->setColor(Primitive::all,geom_red());
  ground->setRollingFriction(0.1f);
  chasis->setColor(Primitive::all,geom_green());

  scene.addObject(ground,true);
  scene.addObject(chasis,true);

  //make the wheels spin
  wheel_cons[0]->setAngularMotor(0,true,150.f,500.f);
  wheel_cons[1]->setAngularMotor(0,true,150.f,500.f);
  wheel_cons[2]->setAngularMotor(0,true,150.f,500.f);
  wheel_cons[3]->setAngularMotor(0,true,150.f,500.f);

  scene.addConstraint(wheel_cons[0],true,true);
  scene.addConstraint(wheel_cons[1],true,true);
  scene.addConstraint(wheel_cons[2],true,true);
  scene.addConstraint(wheel_cons[3],true,true);

  scene.setGravity(Vec(0,0,-9810));

  gui["draw"].install(&handler);
  
  //link the visualization
  gui["draw"].link(scene.getGLCallback(0));
}

int delay = 0;
void run()
{
  //delay the simulation for a second
  if(delay++ < fps.getMaxFPS()*2){
    scene.step(0.f);
  } else {
    scene.step();//0.0016);
  }
  gui["draw"].render();
  fps.wait();
}

int main(int n, char**ppc){
  return ICLApplication(n,ppc,"",init,run).exec();
}
