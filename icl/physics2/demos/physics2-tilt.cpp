// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL physics2 demo: a KINEMATIC tilting platform with balls rolling on it —
// the tilt-maze fix in miniature. Drag the sliders to tilt the platform; the
// collision surface tilts with it (kinematic body), so the balls roll
// correctly. Toggle the collision-shape debug overlay to see physics vs render
// agree.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/geom/Material.h>
#include <icl/geom/Camera.h>
#include <icl/geom2/CuboidNode.h>
#include <icl/geom2/SphereNode.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <icl/geom2/DefaultScene.h>
#include <icl/physics2/PhysicsScene.h>
#include <icl/physics2/RigidBodyDriver.h>
#include <icl/physics2/PhysicsMouseHandler.h>
#include <cmath>

using namespace icl::geom2;
using namespace icl::geom;
using namespace icl::utils;
using namespace icl::qt;
using namespace icl::physics2;

GUI gui;
FPSLimiter fps(60);
PhysicsScene scene;
RigidBodyDriver *platform = nullptr;
Time lastTick;

static Mat rotY(float t){ Mat m=Mat::id(); m(0,0)=cosf(t);m(0,2)=sinf(t);m(2,0)=-sinf(t);m(2,2)=cosf(t); return m; }
static Mat rotX(float t){ Mat m=Mat::id(); m(1,1)=cosf(t);m(1,2)=-sinf(t);m(2,1)=sinf(t);m(2,2)=cosf(t); return m; }

void init() {
  // Default environment + ground collider — the catch floor for balls that
  // roll off the platform.
  scene.setupDefault(DefaultScene::SceneType::Studio, 2000.f);

  // kinematic tilting platform
  auto plat = CuboidNode::create(0,0,0, 1200,1200,30);
  plat->setMaterial(Material::fromColor(GeomColor(70,110,200,255)));
  platform = scene.add(std::static_pointer_cast<Node>(plat), 0.0f);
  platform->setKinematic(true);

  // a handful of balls on the platform
  const GeomColor cols[4] = { {220,60,60,255},{60,200,90,255},{240,200,40,255},{200,80,220,255} };
  float px[4] = {-300, 300, -200, 250}, py[4] = {-250, 200, 300, -300};
  for (int i = 0; i < 4; i++) {
    auto b = SphereNode::create(0,0,0, 55, 28, 28);
    b->setMaterial(Material::fromColor(cols[i]));
    b->translate(px[i], py[i], 200);
    auto d = scene.add(std::static_pointer_cast<Node>(b), 1.0f);
    d->setRollingFriction(0.02f);
    d->setFriction(0.6f);
  }

  scene.start(120);

  gui << (HBox()
          << (VBox().maxSize(14,99).minSize(14,1)
              << ui::FSlider(-0.35f, 0.35f, 0.0f, {.handle="tx", .label="tilt about Y"})
              << ui::FSlider(-0.35f, 0.35f, 0.0f, {.handle="ty", .label="tilt about X"})
              << ui::CheckBox("collision debug", {.handle="dbg"}))
          << ui::Canvas3D(Size(800,600), {.handle="draw"}))
      << ui::Show();

  gui["draw"].link(scene.getGLCallback(0).get());
  static PhysicsMouseHandler handler(0, &scene.scene(), &scene.world());
  gui["draw"].install(&handler);                         // grab: highest priority
  gui["draw"].install(scene.scene().getMouseHandler(0)); // camera nav: installed last
  lastTick = Time::now();
}

void run() {
  Time now = Time::now();
  double dt = (now - lastTick).toSecondsDouble();
  lastTick = now;

  float tx = gui["tx"], ty = gui["ty"];
  platform->setKinematicTransform(rotY(tx) * rotX(ty));
  scene.setDebugDrawEnabled(gui["dbg"]);

  scene.sync(dt);
  gui["draw"].render();
  fps.wait();
}

int main(int n, char **ppc) {
  return ICLApp(n, ppc, "", init, run).exec();
}
