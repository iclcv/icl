// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL physics2 demo: rigid bodies via driver-based physics on geom2.
// Mirrors the legacy physics-scene, but physics runs on its OWN thread
// (PhysicsScene::start) decoupled from rendering — the run() loop only pulls
// the latest poses into the nodes (scene.sync) and draws.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/geom/Material.h>
#include <icl/geom/Camera.h>
#include <icl/geom2/CuboidNode.h>
#include <icl/geom2/CylinderNode.h>
#include <icl/geom2/SphereNode.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <icl/geom2/DefaultScene.h>
#include <icl/physics2/PhysicsScene.h>
#include <icl/physics2/RigidBodyDriver.h>
#include <icl/physics2/PhysicsMouseHandler.h>

using namespace icl::geom2;
using namespace icl::geom;
using namespace icl::utils;
using namespace icl::qt;
using namespace icl::physics2;

GUI gui;
FPSLimiter fps(60);
PhysicsScene scene;
Time lastTick;

// helper: a node placed at (x,y,z) with origin-centred geometry
static std::shared_ptr<Node> at(std::shared_ptr<Node> n, float x, float y, float z) {
  n->translate(x, y, z);
  return n;
}

void init() {
  // Default environment (camera, lamp rig, checkerboard ground) + a matching
  // static ground collider; bodies dropped below rest on the drawn ground.
  scene.setupDefault(DefaultScene::SceneType::Studio, 1000.f);

  // --- falling box ---
  auto box = CuboidNode::create(0,0,0, 100,100,100);
  box->setMaterial(Material::fromColor(GeomColor(60,120,220,255)));
  auto bd = scene.add(at(std::static_pointer_cast<Node>(box), 0,0,500), 0.1f);
  bd->setRestitution(0.5f); bd->setFriction(0.5f); bd->setRollingFriction(0.1f);

  // --- falling cylinder ---
  auto cyl = CylinderNode::create(0,0,0, 100,100,100, 30);
  cyl->setMaterial(Material::fromColor(GeomColor(60,200,90,255)));
  auto cd = scene.add(at(std::static_pointer_cast<Node>(cyl), 0,-20,700), 0.1f);
  cd->setRestitution(0.5f); cd->setFriction(0.5f); cd->setRollingFriction(0.2f);

  // --- falling sphere ---
  auto sph = SphereNode::create(0,0,0, 100, 32, 32);
  sph->setMaterial(Material::fromColor(GeomColor(220,60,60,255)));
  auto sd = scene.add(at(std::static_pointer_cast<Node>(sph), -200,0,300), 0.1f);
  sd->setRestitution(0.5f); sd->setFriction(0.5f); sd->setRollingFriction(0.1f);

  scene.start(120);   // physics on its own thread at 120 Hz

  gui << ui::Canvas3D({.handle="draw"}) << ui::Show();
  gui["draw"].link(scene.getGLCallback(0).get());
  // Shift+Left-drag grabs and drags dynamic bodies; otherwise camera nav.
  static PhysicsMouseHandler handler(0, &scene.scene(), &scene.world());
  gui["draw"].install(&handler);                         // grab: highest priority
  gui["draw"].install(scene.scene().getMouseHandler(0)); // camera nav: installed last

  lastTick = Time::now();
}

void run() {
  Time now = Time::now();
  double dt = (now - lastTick).toSecondsDouble();
  lastTick = now;

  scene.sync(dt);     // pull the latest simulated poses into the nodes
  gui["draw"].render();
  fps.wait();
}

int main(int n, char **ppc) {
  return ICLApp(n, ppc, "", init, run).exec();
}
