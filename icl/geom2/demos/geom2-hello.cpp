// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL geom2 demo: minimal scene with parametric shapes + lighting

#include <icl/qt/Common.h>
#include <icl/geom2/Scene2.h>
#include <icl/geom2/GroupNode.h>
#include <icl/geom2/SphereNode.h>
#include <icl/geom2/CuboidNode.h>
#include <icl/geom2/CylinderNode.h>
#include <icl/geom2/ConeNode.h>
#include <icl/geom2/CoordinateFrameNode.h>
#include <icl/geom2/MeshNode.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom2/TextNode.h>
#include <icl/geom2/Scene2MouseHandler.h>
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

  // Coordinate frame
  scene.addNode(CoordinateFrameNode::create(100, 5));

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

  // Text labels above each shape
  auto labelSphere = TextNode::create("Sphere", 25, GeomColor(255,80,80,255));
  labelSphere->translate(-100, -100, 120);
  scene.addNode(labelSphere);

  auto labelCube = TextNode::create("Cube", 25, GeomColor(80,80,255,255));
  labelCube->translate(120, 0, 80);
  scene.addNode(labelCube);

  auto labelCyl = TextNode::create("Cylinder", 25, GeomColor(80,220,80,255));
  labelCyl->translate(-80, 0, 100);
  scene.addNode(labelCyl);

  auto labelCone = TextNode::create("Cone", 25, GeomColor(255,220,60,255));
  labelCone->translate(-80, 120, 75);
  scene.addNode(labelCone);

  scene.setBounds(400);

  // GUI
  gui << Canvas3D(Size(800, 600)).handle("canvas") << Show();
  gui["canvas"].link(scene.getGLCallback(0).get());
  gui["canvas"].install(scene.getMouseHandler(0));
}

void run() {
  gui["canvas"].render();
}

int main(int argc, char **argv) {
  return ICLApp(argc, argv, "", init, run).exec();
}
