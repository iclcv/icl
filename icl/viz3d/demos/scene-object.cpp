// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Erik Weitnauer

// viz3d port of the legacy geom/scene-object demo: view an .obj file or one of
// a few parametric shapes (cuboid/cylinder/cone/spheroid) or a Lorenz-attractor
// point cloud, with a focal-length slider.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/viz3d/scene/Scene2.h>
#include <icl/viz3d/nodes/MeshNode.h>
#include <icl/viz3d/nodes/CuboidNode.h>
#include <icl/viz3d/nodes/SphereNode.h>
#include <icl/viz3d/nodes/CylinderNode.h>
#include <icl/viz3d/nodes/ConeNode.h>
#include <icl/viz3d/scene/Scene2MouseHandler.h>
#include <icl/viz3d/render/Material.h>
#include <icl/cv3d/Camera.h>

using namespace icl::viz3d;
using namespace icl::cv3d;
using namespace icl::utils;
using namespace icl::qt;

GUI gui;
Scene2 scene;
std::vector<NodePtr> loaded;     // the currently shown object(s), for -o reload

struct LorenzAttractor : public Vec {
  float rho, sigma, beta, dt;
  LorenzAttractor(float rho=28, float sigma=10, float beta=8./3,
                  float x=0.1, float y=0, float z=0, float dt=0.001):
    Vec(x,y,z,1), rho(rho), sigma(sigma), beta(beta), dt(dt) {}
  const Vec &step() {
    float &x = (*this)[0], &y = (*this)[1], &z = (*this)[2];
    float dx = sigma * (y - x);
    float dy = rho * x - y - x*z;
    float dz = x*y - beta * z;
    x += dx*dt; y += dy*dt; z += dz*dt;
    return *this;
  }
};

// build the object(s) selected by the program args, add them to the scene
static void buildObjects() {
  for (auto &n : loaded) scene.removeNode(n.get());
  loaded.clear();

  auto white_red = Material::fromColors(GeomColor(255,255,255,255), GeomColor(255,0,0,255));

  if (pa("-o")) {                                   // load an .obj/.glb/... file
    for (auto &m : MeshNode::load(*pa("-o"))) {
      if (pa("-n")) m->createAutoNormals();
      m->setMaterial(white_red);
      m->setPrimitiveVisible(PrimLine, true);
      scene.addNode(std::static_pointer_cast<Node>(m));
      loaded.push_back(m);
    }
    return;
  }

  const std::string shape = pa("-s").as<std::string>();
  const float d[] = {0,0,0, 7,3,2, 30,30};          // cx,cy,cz, r/extents, slices,stacks
  NodePtr node;
  if (shape == "cuboid") {
    node = CuboidNode::create(d[0],d[1],d[2], d[3],d[4],d[5]);
  } else if (shape == "spheroid") {
    node = std::make_shared<SphereNode>(d[0],d[1],d[2], d[3],d[4],d[5], (int)d[6], (int)d[7]);
  } else if (shape == "cylinder") {
    node = CylinderNode::create(d[0],d[1],d[2], d[3],d[4],d[5], (int)d[6]);
  } else if (shape == "cone") {
    node = ConeNode::create(d[0],d[1],d[2], d[3],d[4],d[5], (int)d[6]);
  } else if (shape == "point-cloud") {
    auto mesh = std::make_shared<MeshNode>();
    LorenzAttractor lorenz;
    for (int i = 0; i < 1000000; ++i) {
      const Vec &v = lorenz.step();
      GeomColor c((v[0]+20.f)*(255.f/41.f), (v[1]+26.7f)*(255.f/55.1f),
                  v[2]*255.f/54.4f, 255);
      mesh->addVertex(Vec(v[0], v[1], v[2]-25, 1), c);
      if (i) mesh->addLine(i, i-1, c);               // line takes the vertex colour
    }
    node = mesh;
  } else {
    pa_show_usage("invalid shape arg for -s");
    ::exit(-1);
  }
  if (auto *g = dynamic_cast<GeometryNode*>(node.get())) {
    if (pa("-n")) g->createAutoNormals();
  }
  scene.addNode(node);
  loaded.push_back(node);
}

void init() {
  gui << (HSplit()
          << Canvas3D({.handle="draw", .label="scene view", .minSize={16, 12}})
          << (VBox().maxSize(12, 99).minSize(10, 1)   // size-limited control panel
              << FSlider(0.5, 20, 3, {.handle="f", .label="focal length"})
              << Button("reload", {.handle="reload", .hide=!pa("-o")})))
      << Show();

  scene.addCamera(Camera(Vec(0,0,-10), Vec(0,0,1), Vec(1,0,0)));
  buildObjects();

  gui["draw"].install(scene.getMouseHandler(0));
  if (pa("-o")) gui["reload"].registerCallback(buildObjects);
  gui["draw"].link(scene.getGLCallback(0).get());
}

void run() {
  scene.getCamera(0).setFocalLength(gui["f"]);
  gui["draw"].render();
  static FPSLimiter limiter(25);
  limiter.wait();
}

int main(int n, char **ppc) {
  pa_explain("-o", "loads a given opengl .obj file (if -o and -s is given, -o is used)");
  pa_explain("-s", "visualizes one of the shape types (cylinder, spheroid, cuboid, cone, point-cloud)");
  return ICLApplication(n, ppc, "-obj|-o(.obj-filename) -shape|-s(shape=cuboid) "
                        "-create-auto-normals|-n", init, run).exec();
}
