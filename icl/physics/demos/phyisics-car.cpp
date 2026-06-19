// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom/Geom.h>
#include <icl/geom/Material.h>
#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>

#include <icl/physics/PhysicsScene2.h>
#include <icl/physics/RigidBoxObject.h>
#include <icl/physics/RigidCylinderObject.h>
#include <icl/physics/RigidSphereObject.h>
#include <icl/physics/HingeConstraint.h>
#include <icl/physics/PhysicsMouseHandler2.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <vector>

using namespace geom;
using namespace physics;

GUI gui;
FPSLimiter fps(60);
Camera cam(Vec(800,0,50), Vec(-1,0.3,0), Vec(0,0,-1));

PhysicsScene2 scene;
PhysicsMouseHandler2 handler(0, &scene.getScene2(), &scene);

// dynamic objects + their initial transforms, for the reset button
std::vector<RigidObject*> dynObjs;
std::vector<Mat> initXforms;
HingeConstraint *wheel_cons[4];

static void addDyn(RigidObject *o){
  dynObjs.push_back(o);
  initXforms.push_back(o->getTransformation());
}

static void applyMotors(){
  // (motor axis, enable, target angular velocity, max motor force).
  // Middle ground: 500 (original) barely drove, 6000 spun the wheels enough to
  // slip the car into a circular drift (it has no differential / steering).
  for(int i=0;i<4;++i) wheel_cons[i]->setAngularMotor(0, true, 150.f, 2500.f);
}

void reset(){
  scene.lock();
  for(size_t i=0;i<dynObjs.size();++i){
    dynObjs[i]->setTransformation(initXforms[i]);
    dynObjs[i]->setLinearVelocity(Vec(0,0,0,0));
    dynObjs[i]->setAngularVelocity(Vec(0,0,0,0));
    dynObjs[i]->activate(true);
  }
  applyMotors();
  scene.unlock();
}

void init(){
  gui << Canvas3D({.handle="draw", .minSize={40,30}})
      << Button("reset", {.handle="reset"})
      << Show();
  scene.addCamera(cam);
  scene.setBounds(2000);  // calibrates camera dolly/pan speed to the scene
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
    addDyn(wheels[i]);
  }

  //add the bricks to the scene — a wall the car drives into
  for(int x = 0; x < 5; x++) {
    for(int z = 0; z < 6; z++) {
      RigidBoxObject *brick = new RigidBoxObject(x*100-450+z%2*20,500,z*40-80, 100, 100, 40, 0.3);
      brick->setFriction(0.8f);
      brick->setDamping(0.1,0.1);
      brick->setRestitution(0.f);
      scene.addObject(brick,true);
      addDyn(brick);
    }
  }

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

  ground->setMaterial(Material::fromColor(geom_red()));
  ground->setRollingFriction(0.1f);
  chasis->setMaterial(Material::fromColor(geom_green()));

  scene.addObject(ground,true);
  scene.addObject(chasis,true);
  addDyn(chasis);

  //make the wheels spin
  applyMotors();

  scene.addConstraint(wheel_cons[0],true,true);
  scene.addConstraint(wheel_cons[1],true,true);
  scene.addConstraint(wheel_cons[2],true,true);
  scene.addConstraint(wheel_cons[3],true,true);

  scene.setGravity(Vec(0,0,-9810));

  gui["draw"].install(&handler);
  gui["draw"].install(scene.getScene2().getMouseHandler(0));
  gui["reset"].registerCallback(reset);

  //link the visualization
  gui["draw"].link(scene.getGLCallback(0).get());
}

int delay = 0;
void run()
{
  //let the car settle on its wheels for ~1s, then drive
  if(delay++ < fps.getMaxFPS()){
    scene.step(0.f);
  } else {
    scene.step();
  }
  scene.syncSceneFromPhysics();
  gui["draw"].render();
  fps.wait();
}

int main(int n, char**ppc){
  return ICLApplication(n,ppc,"",init,run).exec();
}
