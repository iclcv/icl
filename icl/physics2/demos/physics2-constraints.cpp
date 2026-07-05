// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL physics2 demo: constraints (joints) — the physics2 port of the legacy
// physics-constraints demo. Four stations exercise every joint factory:
//   - HINGE       : a door swings about a horizontal axis (axis X)
//   - BALLSOCKET  : a pendulum swings freely below its anchor
//   - SLIDER      : a weight slides along one axis (vertical here)
//   - SPRING      : a box is tethered to a point and springs back when disturbed
// Shift+drag grabs a body (PhysicsMouseHandler); plain drag orbits the camera.
// Toggle the collision-shape overlay to see physics vs render agree.
//
// Runs in the default unified (Deformable) world — it hosts rigid bodies + 6DOF
// joints (+ deformable cloth) in one solver. NB the only joint-axis caveat:
// btGeneric6DofConstraint gimbal-limits the middle (Y) angular axis, so hinges
// here are about X or Z.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/geom/Material.h>
#include <icl/viz3d/CuboidNode.h>
#include <icl/viz3d/SphereNode.h>
#include <icl/viz3d/Scene2MouseHandler.h>
#include <icl/viz3d/DefaultScene.h>
#include <icl/physics2/PhysicsScene.h>
#include <icl/physics2/RigidBodyDriver.h>
#include <icl/physics2/Constraint.h>
#include <icl/physics2/PhysicsMouseHandler.h>

using namespace icl::viz3d;
using namespace icl::geom;
using namespace icl::utils;
using namespace icl::qt;
using namespace icl::physics2;

GUI gui;
FPSLimiter fps(60);
PhysicsScene scene;   // default unified (Deformable) world: rigid + joints + cloth
Time lastTick;

// A visible static anchor post (a slim grey box), returned as a NodePtr.
static std::shared_ptr<CuboidNode> addPost(float x, float y, float z) {
  auto p = CuboidNode::create(0, 0, 0, 50, 50, 50);
  p->setMaterial(Material::fromColor(GeomColor(150, 150, 150, 255)));
  p->translate(x, y, z);
  scene.add(std::static_pointer_cast<Node>(p), 0.0f);   // static
  return p;
}

void init() {
  scene.setupDefault(DefaultScene::SceneType::Studio, 1600.f);

  const float Z = 700;     // anchor height; bodies hang/settle below it

  // --- HINGE: door swings about X (axis 0). Anchor offset in +Y, out of the
  //     YZ swing plane; door extends +Y from the pivot at (-500,0,Z). ---
  auto hPost = addPost(-500, 300, Z);
  auto door = CuboidNode::create(0, 0, 0, 60, 360, 240);
  door->setMaterial(Material::fromColor(GeomColor(70, 110, 200, 255)));
  door->translate(-500, 180, Z);                          // -Y edge at the pivot
  auto dd = scene.add(std::static_pointer_cast<Node>(door), 1.0f);
  dd->setDamping(0.05f, 0.2f);
  scene.addHinge(hPost, door, Vec(0, -300, 0, 1), Vec(0, -180, 0, 1), 0);

  // --- BALLSOCKET: a pendulum bob swings freely below its anchor. ---
  auto bPost = addPost(-150, 0, Z);
  auto bob = SphereNode::create(0, 0, 0, 70, 28, 28);
  bob->setMaterial(Material::fromColor(GeomColor(60, 200, 90, 255)));
  bob->translate(-150, 0, Z - 250);                       // hangs 250 below
  auto bd = scene.add(std::static_pointer_cast<Node>(bob), 1.0f);
  bd->setDamping(0.05f, 0.1f);
  scene.addBallSocket(bPost, bob, Vec(0, 0, 0, 1), Vec(0, 0, 250, 1));

  // --- SLIDER: a weight free to slide along Z only, hung off a post so it
  //     bottoms out under gravity; grab it to lift it. ---
  auto sPost = addPost(150, 0, Z);
  auto weight = CuboidNode::create(0, 0, 0, 160, 160, 80);
  weight->setMaterial(Material::fromColor(GeomColor(240, 200, 40, 255)));
  weight->translate(150, 0, Z - 150);
  scene.add(std::static_pointer_cast<Node>(weight), 1.0f);
  scene.addSlider(sPost, weight, Vec(0, 0, 0, 1), Vec(0, 0, 150, 1), 2);

  // --- SPRING: a box tethered to a world point; springs back when grabbed. ---
  auto box = CuboidNode::create(0, 0, 0, 150, 150, 150);
  box->setMaterial(Material::fromColor(GeomColor(200, 80, 220, 255)));
  box->translate(500, 0, Z - 200);
  auto pd = scene.add(std::static_pointer_cast<Node>(box), 1.0f);
  pd->setDamping(0.4f, 0.4f);
  scene.addSpring(box, Vec(0, 0, 0, 1), Vec(500, 0, Z - 200, 1), 2000.f, 0.9f);

  scene.start(120);

  gui << (HBox()
          << (VBox().maxSize(14, 99).minSize(14, 1)
              << CheckBox("collision debug", {.handle = "dbg"}))
          << Canvas3D(Size(900, 650), {.handle = "draw"}))
      << Show();

  gui["draw"].link(scene.getGLCallback(0).get());
  static PhysicsMouseHandler handler(0, &scene.scene(), &scene.world());
  gui["draw"].install(&handler);                          // grab: highest priority
  gui["draw"].install(scene.scene().getMouseHandler(0));  // camera nav: last
  lastTick = Time::now();
}

void run() {
  Time now = Time::now();
  double dt = (now - lastTick).toSecondsDouble();
  lastTick = now;

  scene.setDebugDrawEnabled(gui["dbg"]);
  scene.sync(dt);
  gui["draw"].render();
  fps.wait();
}

int main(int n, char **ppc) {
  return ICLApp(n, ppc, "", init, run).exec();
}
