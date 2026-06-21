// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL physics2 demo: driving game — MILESTONE 1 (VehicleDriver shake-out).
// A raycast-vehicle car on a flat ground, driven by sliders, viewed from a
// fixed camera. M2 replaces the sliders with keyboard input (WASD) and the
// fixed camera with a chase camera. Runs in the default unified Deformable
// world (rigid + the vehicle action together).
//
//   throttle : forward/reverse engine force
//   steer    : front-wheel angle
//   brake    : all-wheel brake

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/geom/Material.h>
#include <icl/geom/Camera.h>
#include <icl/geom2/CuboidNode.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <icl/geom2/DefaultScene.h>
#include <icl/physics2/PhysicsScene.h>
#include <icl/physics2/VehicleDriver.h>

using namespace icl::geom2;
using namespace icl::geom;
using namespace icl::utils;
using namespace icl::qt;
using namespace icl::physics2;

GUI gui;
FPSLimiter fps(60);
PhysicsScene scene;          // default unified (Deformable) world
VehicleDriver *car = nullptr;
Time lastTick;

void init() {
  scene.setupDefault(DefaultScene::SceneType::Studio, 4000.f);

  auto body = CuboidNode::create(0, 0, 0, 600, 1200, 300);
  body->setMaterial(Material::fromColor(GeomColor(200, 60, 60, 255)));
  body->translate(0, 0, -300);     // a little above the ground; settles onto its wheels
  car = scene.addVehicle(std::static_pointer_cast<Node>(body));

  scene.start(120);

  gui << (HBox()
          << (VBox().maxSize(14, 99).minSize(14, 1)
              << FSlider(-30000, 30000, 0, {.handle = "throttle", .label = "throttle"})
              << FSlider(-0.5f, 0.5f, 0.0f, {.handle = "steer", .label = "steer"})
              << FSlider(0, 5000, 0, {.handle = "brake", .label = "brake"})
              << CheckBox("collision debug", {.handle = "dbg"}))
          << Canvas3D(Size(900, 650), {.handle = "draw"}))
      << Show();

  gui["draw"].link(scene.getGLCallback(0).get());
  gui["draw"].install(scene.scene().getMouseHandler(0));   // free camera nav for now
  lastTick = Time::now();
}

void run() {
  Time now = Time::now();
  double dt = (now - lastTick).toSecondsDouble();
  lastTick = now;

  car->setEngineForce(gui["throttle"]);
  car->setSteering(gui["steer"]);
  car->setBrake(gui["brake"]);
  scene.setDebugDrawEnabled(gui["dbg"]);

  scene.sync(dt);
  gui["draw"].render();
  fps.wait();
}

int main(int n, char **ppc) {
  return ICLApp(n, ppc, "", init, run).exec();
}
