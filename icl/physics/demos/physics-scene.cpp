// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom/Geom.h>
#include <icl/geom/Material.h>
#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>

#include <icl/physics/PhysicsScene2.h>
#include <icl/physics/PhysicsMouseHandler2.h>
#include <icl/physics/RigidBoxObject.h>
#include <icl/physics/RigidCylinderObject.h>
#include <icl/physics/RigidSphereObject.h>

using namespace geom;
using namespace physics;

GUI gui;
FPSLimiter fps(60);
Camera cam(Vec(2000,0,500,1), Vec(-1,0,0,1), Vec(0,0,-1,1));

RigidBoxObject box(0,0,500, 100, 100, 100, 0.1);
RigidCylinderObject cylinder(0,-20,700.0, 100, 100 , 0.1);
RigidSphereObject sphere(-200,0,90,100,0.1);
RigidBoxObject table(0,0,-200.0, 10000, 10000, 200, 0);
PhysicsScene2 scene;  // simulates in a PhysicsWorld, renders via geom2
PhysicsMouseHandler2 handler(0, &scene.getScene2(), &scene);

void init(){
  gui << ui::Canvas3D({.handle="draw"}) << ui::Show();
  scene.addCamera(cam);
  scene.setBounds(3000);  // calibrates camera dolly/pan speed to the scene

  table.setMaterial(Material::fromColor(geom_red()));

  cylinder.setRestitution(0.5f);
  sphere.setRestitution(0.5f);
  box.setRestitution(0.5f);
  table.setRestitution(0.9f);

  box.setFriction(0.5f);
  cylinder.setFriction(0.5f);
  sphere.setFriction(0.5f);
  table.setFriction(0.5f);

  cylinder.setRollingFriction(0.2f);
  sphere.setRollingFriction(0.1f);
  table.setRollingFriction(0.2f);
  box.setRollingFriction(0.1f);

  scene.addObject(&cylinder);
  scene.addObject(&sphere);
  scene.addObject(&box);
  scene.addObject(&table);

  // camera navigation + Shift+Left-drag object grabbing
  gui["draw"].link(scene.getGLCallback(0).get());
  gui["draw"].install(&handler);
}

void run()
{
  scene.step();
  scene.syncSceneFromPhysics();
  gui["draw"].render();
  fps.wait();
}

int main(int n, char**ppc){
  return ICLApplication(n,ppc,"",init,run).exec();
}
