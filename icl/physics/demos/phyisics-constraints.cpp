// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom/Geom.h>
#include <icl/geom/Material.h>
#include <icl/qt/Common2.h>
#include <icl/utils/FPSLimiter.h>

#include <icl/physics/PhysicsScene.h>
#include <icl/physics/RigidBoxObject.h>
#include <icl/physics/RigidCylinderObject.h>
#include <icl/physics/RigidSphereObject.h>
#include <icl/physics/HingeConstraint.h>
#include <icl/physics/PhysicsMouseHandler.h>
using namespace geom;
using namespace physics;

GUI gui;
FPSLimiter fps(60);
Camera cam(Vec(2000,0,500,1), Vec(-1,0,0,1), Vec(0,0,-1,1));


PhysicsScene scene;

PhysicsMouseHandler handler(0,&scene);

void init(){
  gui << Canvas3D().handle("draw") << Show();
  scene.addCamera(cam);

  RigidBoxObject *door = new RigidBoxObject(0,115,200, 20, 200, 390, 0.1);
  RigidCylinderObject *hinge = new RigidCylinderObject(0,0,200,20,400, 0.0);
  RigidBoxObject *ground = new RigidBoxObject(0.0,0.0,-10, 1000, 1000, 20, 0);
  HingeConstraint *hinge_constraint = new HingeConstraint(hinge, door, Vec(0,0,0), Vec(0,-115,0), 2);

  ground->setMaterial(Material::fromColor(geom_red()));

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
