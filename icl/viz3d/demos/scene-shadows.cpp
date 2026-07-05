// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Erik Weitnauer, Matthias Esau

// viz3d port of the legacy geom/scene-shadows demo: shadow-casting point lights
// orbiting an object (a loaded .obj or a parametric shape) above a plane.
// viz3d LightNodes carry shadows natively (setShadowEnabled / soft-shadow
// radius); their position comes from the node transform, animated each frame.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/viz3d/scene/Scene2.h>
#include <icl/viz3d/nodes/MeshNode.h>
#include <icl/viz3d/nodes/CuboidNode.h>
#include <icl/viz3d/nodes/SphereNode.h>
#include <icl/viz3d/nodes/CylinderNode.h>
#include <icl/viz3d/nodes/ConeNode.h>
#include <icl/viz3d/nodes/LightNode.h>
#include <icl/viz3d/scene/Scene2MouseHandler.h>
#include <icl/viz3d/render/Material.h>
#include <icl/cv3d/Camera.h>

using namespace icl::viz3d;
using namespace icl::cv3d;
using namespace icl::utils;
using namespace icl::qt;

GUI gui;
Scene2 scene;
unsigned int lights;
std::vector<NodePtr> shapeNodes;   // the loaded/created object(s), for reload

static void applyObjStyle(const std::shared_ptr<GeometryNode> &o) {
  o->setMaterial(Material::fromColors(GeomColor(255,255,255,255), GeomColor(255,0,0,255)));
  if (!(bool)pa("-render-lines"))  o->setPrimitiveVisible(PrimLine,   false);
  if (!(bool)pa("-render-points")) o->setPrimitiveVisible(PrimVertex, false);
}

static void buildShape() {
  for (auto &n : shapeNodes) scene.removeNode(n.get());
  shapeNodes.clear();

  const float so = pa("-so");
  if (pa("-o")) {                                  // load an .obj file
    for (auto &m : MeshNode::load(*pa("-o"))) {
      m->scale(so, so, so);
      if (pa("-n")) m->createAutoNormals();
      applyObjStyle(m);
      scene.addNode(std::static_pointer_cast<Node>(m));
      shapeNodes.push_back(m);
    }
  } else {                                         // or a parametric shape
    const std::string s = pa("-s").as<std::string>();
    const float d[] = {0,0,0, 7,3,2, 30,30};
    std::shared_ptr<GeometryNode> o;
    if      (s == "cuboid")   o = CuboidNode::create(d[0],d[1],d[2], d[3],d[4],d[5]);
    else if (s == "spheroid") o = std::make_shared<SphereNode>(d[0],d[1],d[2], d[3],d[4],d[5], (int)d[6],(int)d[7]);
    else if (s == "cylinder") o = CylinderNode::create(d[0],d[1],d[2], d[3],d[4],d[5], (int)d[6]);
    else if (s == "cone")     o = ConeNode::create(d[0],d[1],d[2], d[3],d[4],d[5], (int)d[6]);
    else { pa_show_usage("invalid shape arg for -s"); ::exit(-1); }
    if (pa("-n")) o->createAutoNormals();
    applyObjStyle(o);
    scene.addNode(std::static_pointer_cast<Node>(o));
    shapeNodes.push_back(o);
  }
}

void init() {
  gui << Canvas3D({.handle="draw", .label="scene view", .minSize={16, 12}})
      << ( HBox({.maxSize={99, 3}})
           << FSlider(0.5, 20, 3, {.handle="f", .label="focal length", .maxSize={100, 3}})
           << FSlider(1, 100, 15, {.handle="r", .label="light radius", .maxSize={100, 3}})
           << Button("reload", {.handle="reload", .hide=!(bool)pa("-o")}) )
      << Show();

  scene.addCamera(Camera(Vec(0,0,-10), Vec(0,0,1), Vec(1,0,0)));

  // a plane + a floating box to catch/cast shadows
  scene.addNode(CuboidNode::create(4, 0, 0, 1, 30, 30));
  scene.addNode(CuboidNode::create(0, 6, 6, 10, 4, 4));

  lights = pa("-l");
  for (unsigned int i = 0; i < lights; i++) {
    auto l = std::make_shared<LightNode>(LightNode::Point);
    l->setIntensity(1.0f / lights);
    l->setShadowEnabled(true);
    l->setSoftShadowRadius(2.0f);
    l->translate(-4, 10, -30);
    scene.addLight(l);
  }

  buildShape();

  gui["draw"].install(scene.getMouseHandler(0));
  if (pa("-o")) gui["reload"].registerCallback(buildShape);
  gui["draw"].link(scene.getGLCallback(0).get());
}

float timer = 0.f;
void run() {
  scene.lock();
  timer += 0.05f;
  const float r = gui["r"];
  const float h = -10;
  for (unsigned int i = 0; i < lights; i++) {
    auto *l = scene.getLight(i);
    l->removeTransformation();
    l->translate(h, r * std::cos(timer / float(i + 1)), r * std::sin(timer / float(i + 1)));
  }
  scene.getCamera(0).setFocalLength(gui["f"]);
  scene.unlock();

  gui["draw"].render();
  static FPSLimiter limiter(pa("-fps").as<float>());
  limiter.wait();
}

int main(int n, char **ppc) {
  pa_explain("-o", "loads a given opengl .obj file (if -o and -s is given, -o is used)");
  pa_explain("-s", "visualizes one of the shape types (cylinder, spheroid, cuboid, cone)");
  pa_explain("-l", "sets the number of lights in the scene");
  return ICLApplication(n, ppc, "-obj|-o(.obj-filename) -shape|-s(shape=cuboid) "
                        "-lights|-l(num=1) -create-auto-normals|-n -scale-object|-so(scale=1) "
                        "-render-lines -render-points -max-fps|-fps(fps=25)", init, run).exec();
}
