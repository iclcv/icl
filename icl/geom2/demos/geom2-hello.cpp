// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL geom2 demo: minimal scene with parametric shapes + lighting

#include <icl/qt/Common.h>
#include <icl/geom2/Scene2.h>
#include <icl/geom2/GroupNode.h>
#include <icl/geom2/SphereNode.h>
#include <icl/geom2/CuboidNode.h>
#include <icl/geom2/CylinderNode.h>
#include <icl/geom2/ConeNode.h>
#include <icl/geom2/MeshNode.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom/Material.h>
#include <icl/geom/Camera.h>

using namespace icl::geom2;
using namespace icl::geom;
using namespace icl::utils;
using namespace icl::qt;

GUI gui;
Scene2 scene;

void init() {
  // Camera
  scene.addCamera(Camera::lookAt(
      Vec(400, 300, 400, 1),
      Vec(0, 0, 30, 1),
      Vec(0, 0, 1, 1),
      Size(800, 600), 60.0f));

  // Red sphere
  auto sphere = SphereNode::create(-100, -100, 50, 50, 40, 40);
  sphere->setMaterial(Material::fromColor(GeomColor(220, 60, 60, 255)));
  sphere->getMaterial()->roughness = 0.3f;
  scene.addNode(sphere);

  // Blue cube
  auto cube = CuboidNode::create(120, 0, 30, 60, 60, 60);
  cube->setMaterial(Material::fromColor(GeomColor(60, 60, 220, 255)));
  cube->getMaterial()->metallic = 0.8f;
  cube->getMaterial()->roughness = 0.2f;
  scene.addNode(cube);

  // Green cylinder
  auto cyl = CylinderNode::create(-80, 0, 40, 30, 30, 80, 30);
  cyl->setMaterial(Material::fromColor(GeomColor(60, 200, 60, 255)));
  scene.addNode(cyl);

  // Yellow cone
  auto cone = ConeNode::create(-80, 120, 30, 40, 40, 60, 20);
  cone->setMaterial(Material::fromColor(GeomColor(240, 200, 40, 255)));
  scene.addNode(cone);

  // Coordinate frame as GroupNode
  auto frame = std::make_shared<GroupNode>();
  // X axis (red): cylinder along Z, rotated 90° around Y
  auto xAxis = CylinderNode::create(0, 0, 0, 3, 3, 100, 12);
  xAxis->setMaterial(Material::fromColor(GeomColor(255, 0, 0, 255)));
  xAxis->rotate(0, M_PI/2, 0);
  xAxis->translate(50, 0, 0);
  frame->addChild(xAxis);
  // Y axis (green): cylinder along Z, rotated -90° around X
  auto yAxis = CylinderNode::create(0, 0, 0, 3, 3, 100, 12);
  yAxis->setMaterial(Material::fromColor(GeomColor(0, 255, 0, 255)));
  yAxis->rotate(-M_PI/2, 0, 0);
  yAxis->translate(0, 50, 0);
  frame->addChild(yAxis);
  // Z axis (blue): cylinder already along Z
  auto zAxis = CylinderNode::create(0, 0, 50, 3, 3, 100, 12);
  zAxis->setMaterial(Material::fromColor(GeomColor(0, 0, 255, 255)));
  frame->addChild(zAxis);
  scene.addNode(frame);

  // Ground plane (freeform MeshNode)
  auto ground = std::make_shared<MeshNode>();
  float gs = 200;
  ground->addVertex(Vec(-gs, -gs, 0, 1));
  ground->addVertex(Vec(gs, -gs, 0, 1));
  ground->addVertex(Vec(gs, gs, 0, 1));
  ground->addVertex(Vec(-gs, gs, 0, 1));
  ground->addQuad(0, 1, 2, 3);
  ground->createAutoNormals(false);
  ground->setMaterial(Material::fromColor(GeomColor(180, 180, 180, 255)));
  scene.addNode(ground);

  // Light
  auto light = std::make_shared<LightNode>(LightNode::Point);
  light->setColor(GeomColor(1.0f, 0.97f, 0.92f, 1.0f));
  light->setIntensity(1.5f);
  light->translate(200, 150, 300);
  scene.addLight(light);

  // GUI
  gui << Canvas3D(Size(800, 600)).handle("canvas") << Show();
  gui["canvas"].link(scene.getGLCallback(0).get());
}

void run() {
  gui["canvas"].render();
}

int main(int argc, char **argv) {
  return ICLApp(argc, argv, "", init, run).exec();
}
