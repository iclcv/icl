// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL physics2 demo: soft bodies. A cloth patch drapes over a static box
// (soft-rigid collision), and a banner hangs pinned by two corners. Physics
// runs on its own thread; the run loop only pulls the simulated mesh into the
// scene (scene.sync). Toggle the collision-shape debug overlay.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/geom/Material.h>
#include <icl/geom/Camera.h>
#include <icl/geom2/CuboidNode.h>
#include <icl/geom2/MeshNode.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <icl/physics2/PhysicsScene.h>
#include <icl/physics2/RigidBodyDriver.h>
#include <icl/physics2/SoftBodyDriver.h>

using namespace icl::geom2;
using namespace icl::geom;
using namespace icl::utils;
using namespace icl::qt;
using namespace icl::physics2;

GUI gui;
FPSLimiter fps(60);
PhysicsScene scene;
Time lastTick;

void init() {
  scene.addCamera(Camera::lookAt(Vec(1400,1000,900,1), Vec(0,0,100,1), Vec(0,0,1,1),
                                 Size(800,600), 50));
  scene.setBounds(2000);

  // ground
  auto ground = CuboidNode::create(0,0,0, 6000,6000,40);
  ground->setMaterial(Material::fromColor(GeomColor(110,110,110,255)));
  ground->translate(0,0,-300);
  scene.add(std::static_pointer_cast<Node>(ground), 0.0f);

  // a static box the cloth will drape over (soft-rigid collision)
  auto box = CuboidNode::create(0,0,0, 400,400,400);
  box->setMaterial(Material::fromColor(GeomColor(180,140,70,255)));
  box->translate(-250,0,-100);
  scene.add(std::static_pointer_cast<Node>(box), 0.0f);

  // cloth dropped flat above the box -> drapes over it
  auto cloth = scene.addCloth(Vec(-650,-450,500,1), Vec(150,-450,500,1),
                              Vec(-650, 450,500,1), Vec(150, 450,500,1),
                              24, 24, /*pin*/ 0, 2.0f);
  dynamic_cast<MeshNode*>(cloth->node())
      ->setMaterial(Material::fromColor(GeomColor(70,150,220,255)));

  // a banner hanging from two pinned corners
  auto banner = scene.addCloth(Vec(500,-400,600,1), Vec(500,400,600,1),
                               Vec(500,-400,-100,1), Vec(500,400,-100,1),
                               20, 20, /*pin c00 + c10*/ 1 + 2, 1.0f);
  dynamic_cast<MeshNode*>(banner->node())
      ->setMaterial(Material::fromColor(GeomColor(220,80,80,255)));

  auto light = std::make_shared<LightNode>(LightNode::Point);
  light->setIntensity(0.9f);
  light->translate(700, 500, 1400);
  light->setShadowEnabled(true);
  scene.addLight(light);

  scene.start(120);

  gui << (HBox()
          << (VBox().maxSize(13,99).minSize(13,1)
              << ui::CheckBox("collision debug", {.handle="dbg"}))
          << ui::Canvas3D(Size(800,600), {.handle="draw"}))
      << ui::Show();
  gui["draw"].link(scene.getGLCallback(0).get());
  gui["draw"].install(scene.getMouseHandler(0));
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
