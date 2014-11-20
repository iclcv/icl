/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/demos/physics-scene/physics-scene.cpp       **
** Module : ICLPhysics                                             **
** Author : Matthias Esau                                          **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/
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

RigidBoxObject box(0,0,500, 100, 100, 100, 0.1);
RigidCylinderObject cylinder(0,-20,700.0, 100, 100 , 0.1);
RigidSphereObject sphere(-200,0,90,100,0.1);
RigidBoxObject table(0,0,-200.0, 10000, 10000, 200, 0);
PhysicsScene scene;

PhysicsMouseHandler handler(0,&scene);

void init(){
  gui << Draw3D().handle("draw") << Show();
  scene.addCamera(cam);

  table.setColor(Primitive::all,geom_red());

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
