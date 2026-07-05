// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL physics2 demo: soft bodies. A cloth patch drapes over a static box
// (soft-rigid collision), and a banner hangs pinned by two corners. Physics
// runs on its own thread; the run loop only pulls the simulated mesh into the
// scene (scene.sync). Toggle the collision-shape debug overlay.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/geom/Material.h>
#include <icl/cv3d/Camera.h>
#include <icl/geom2/CuboidNode.h>
#include <icl/geom2/MeshNode.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <icl/geom2/DefaultScene.h>
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
SoftBodyDriver *cloth = nullptr, *banner = nullptr;

void init() {
  // Default environment + ground collider. ext=1200 -> ground top at z=-624;
  // the content below is placed relative to that (the box rests on it).
  scene.setupDefault(DefaultScene::SceneType::Studio, 1200.f);

  // a static box the cloth will drape over (soft-rigid collision), resting on
  // the default ground (bottom at z=-624 -> centre at -424)
  auto box = CuboidNode::create(0,0,0, 400,400,400);
  box->setMaterial(Material::fromColor(GeomColor(180,140,70,255)));
  box->translate(-250,0,-424);
  scene.add(std::static_pointer_cast<Node>(box), 0.0f);

  // cloth dropped flat just above the box top (z=-224) -> gently settles and
  // drapes. A low drop keeps impact velocity below the collision margin (a high
  // drop tunnels the cloth through the box).
  cloth = scene.addCloth(Vec(-650,-450,-180,1), Vec(150,-450,-180,1),
                         Vec(-650, 450,-180,1), Vec(150, 450,-180,1),
                         50, 50, /*pin*/ 0, 2.0f);
  dynamic_cast<MeshNode*>(cloth->node())
      ->setMaterial(Material::fromColor(GeomColor(70,150,220,255), 128, 0.25f));  // 25% reflective
  cloth->setPropertyValue("size", 1.4f);        // tuned oversize (drapes well, stays put)
  cloth->setPropertyValue("stiffness", 0.85f);  // tuned: heavier 2.0-mass cloth holds shape

  // a banner hanging from two pinned corners
  banner = scene.addCloth(Vec(1000,-400,276,1), Vec(1000,400,276,1),
                          Vec(1000,-400,-424,1), Vec(1000,400,-424,1),
                          20, 20, /*pin c00 + c10*/ 1 + 2, 1.0f);
  dynamic_cast<MeshNode*>(banner->node())
      ->setMaterial(Material::fromColor(GeomColor(220,80,80,255)));

  scene.start(240);   // smaller fixed timestep -> less soft-body tunneling

  // Live soft-body tuning panel (stiffness / friction / margin / collision mode
  // / self collision); changes apply to the running cloth via the sim queue.
  gui << (HSplit()
          << Canvas3D(Size(2400,1800), {.handle="draw", .minSize={32,24}})
          << (VBox().maxSize(16,99)
              << Prop(cloth, {.label="cloth"})
              << Button("reset", {.handle="reset"})
              << CheckBox("collision debug", {.handle="dbg"})))
      << Show();
  gui["draw"].link(scene.getGLCallback(0).get());
  gui["draw"].install(scene.getMouseHandler(0));
  lastTick = Time::now();
}

void run() {
  Time now = Time::now();
  double dt = (now - lastTick).toSecondsDouble();
  lastTick = now;

  static ButtonHandle reset = gui["reset"];
  if (reset.wasTriggered()) { cloth->reset(); banner->reset(); }

  scene.setDebugDrawEnabled(gui["dbg"]);
  scene.sync(dt);
  gui["draw"].render();
  fps.wait();
}

int main(int n, char **ppc) {
  return ICLApp(n, ppc, "", init, run).exec();
}
