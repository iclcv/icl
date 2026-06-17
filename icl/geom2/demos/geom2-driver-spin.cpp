// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL geom2 demo: the Driver mechanism — world-free SpinDrivers animating
// nodes via Scene2::sync(), fully decoupled from rendering.
//
// The motion is made *visible* on purpose:
//   - a "turntable" GroupNode spins about Z; an arm + ball offset from the
//     axis ORBIT the origin (proves parent-before-child sync composition);
//   - a lone cone tumbles about its own X axis (asymmetric → spin is obvious);
//   - a lone cube spins about Z (its corners sweep against the coord frame).

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/geom2/Scene2.h>
#include <icl/geom2/Driver.h>
#include <icl/geom2/GroupNode.h>
#include <icl/geom2/CuboidNode.h>
#include <icl/geom2/SphereNode.h>
#include <icl/geom2/ConeNode.h>
#include <icl/geom2/CoordinateFrameNode.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <icl/geom/Material.h>
#include <icl/geom/Camera.h>

using namespace icl::geom2;
using namespace icl::geom;
using namespace icl::utils;
using namespace icl::qt;

/// Minimal world-free driver: spins its node about a chosen axis at a fixed
/// angular rate, time-driven so the visible speed is independent of framerate.
class SpinDriver : public Driver {
public:
  SpinDriver(float radPerSec, char axis = 'z')
    : m_rate(radPerSec), m_axis(axis) {}

  void sync(double dt, double /*alpha*/) override {
    if (auto *n = node()) {
      float a = m_rate * (float)dt;
      n->rotate(m_axis == 'x' ? a : 0, m_axis == 'y' ? a : 0, m_axis == 'z' ? a : 0);
    }
  }

private:
  float m_rate;
  char  m_axis;
};

GUI gui;
Scene2 scene;
Time lastTick;

void init() {
  scene.addCamera(Camera::lookAt(
      Vec(450, 350, 420, 1), Vec(0, 0, 40, 1), Vec(0, 0, 1, 1),
      Size(800, 600), 60.0f));

  scene.addNode(CoordinateFrameNode::create(120, 2.5, true));

  // --- Turntable: a group spinning about Z. Its children sit OFF the axis,
  //     so the parent spin makes them orbit the origin (visible motion). ---
  auto turntable = std::make_shared<GroupNode>();
  turntable->addDriver<SpinDriver>(0.7f, 'z');

  auto arm = CuboidNode::create(70, 0, 12, 130, 14, 10);   // bar along +x
  arm->setMaterial(Material::fromColor(GeomColor(60, 120, 220, 255)));
  arm->getMaterial()->metallic = 0.7f;
  arm->getMaterial()->roughness = 0.25f;
  turntable->addChild(std::static_pointer_cast<Node>(arm));

  auto ball = SphereNode::create(140, 0, 12, 18, 32, 32);  // at the arm's end
  ball->setMaterial(Material::fromColor(GeomColor(220, 60, 60, 255)));
  turntable->addChild(std::static_pointer_cast<Node>(ball));
  scene.addNode(turntable);

  // --- Lone cone tumbling about its own X axis (asymmetric → spin visible) ---
  auto cone = ConeNode::create(-150, -130, 50, 32, 32, 80, 28);
  cone->setMaterial(Material::fromColor(GeomColor(240, 200, 40, 255)));
  cone->addDriver<SpinDriver>(2.2f, 'x');
  scene.addNode(cone);

  // --- Lone cube spinning about Z (corners sweep against the frame) ---
  auto cube = CuboidNode::createCube(-150, 140, 35, 70);
  cube->setMaterial(Material::fromColor(GeomColor(60, 200, 90, 255)));
  cube->getMaterial()->roughness = 0.4f;
  cube->addDriver<SpinDriver>(1.0f, 'z');
  scene.addNode(cube);

  auto light = std::make_shared<LightNode>(LightNode::Point);
  light->setIntensity(0.85f);
  light->translate(250, 180, 350);
  light->setShadowEnabled(true);
  scene.addLight(light);

  scene.setBounds(450);

  gui << ui::Canvas3D(Size(800, 600), {.handle="canvas"}) << ui::Show();
  gui["canvas"].link(scene.getGLCallback(0).get());
  gui["canvas"].install(scene.getMouseHandler(0));

  lastTick = Time::now();
}

void run() {
  Time now = Time::now();
  double dt = (now - lastTick).toSecondsDouble();
  lastTick = now;

  scene.sync(dt);              // advance all drivers (UI thread)
  gui["canvas"].render();
}

int main(int argc, char **argv) {
  return ICLApp(argc, argv, "", init, run).exec();
}
