// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL geom2 demo: DefaultScene presets (Studio / Void), live-configurable.
// A handful of shapes dropped into a self-furnishing scene; the Prop panel
// switches preset, up-axis, and toggles sky / shadows / SSR / ground at runtime.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/geom2/DefaultScene.h>
#include <icl/geom2/SphereNode.h>
#include <icl/geom2/CuboidNode.h>
#include <icl/geom2/CylinderNode.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <icl/geom/Material.h>

using namespace icl::geom2;
using namespace icl::geom;
using namespace icl::utils;
using namespace icl::qt;

GUI gui;
DefaultScene scene(DefaultScene::SceneType::Studio);

void init() {
  // Objects rest ON the ground. Default extent 400 → ground level at
  // -half - 2% = -208 (Y-up); place each shape's base there.
  const float groundY = -208.0f;

  auto sphere = SphereNode::create(120, groundY + 60, 0, 60, 40, 40);
  sphere->setMaterial(Material::fromColor(GeomColor(220, 60, 60, 255)));
  sphere->getMaterial()->roughness = 0.15f;
  sphere->getMaterial()->reflectivity = 0.6f;
  scene.addNode(sphere);

  auto cube = CuboidNode::create(-120, groundY + 45, 0, 90, 90, 90);
  cube->setMaterial(Material::fromColor(GeomColor(60, 90, 220, 255)));
  cube->getMaterial()->metallic = 0.8f;
  cube->getMaterial()->roughness = 0.2f;
  scene.addNode(cube);

  // Z-aligned cylinder lies on its side → rests one radius (40) above ground.
  // Give it reflectivity so the flat cap reflects too (a non-reflective dielectric
  // shows ~0 reflection head-on — Fresnel only kicks in at grazing angles, which
  // is why a bare-color cap reads as flat diffuse green).
  auto cyl = CylinderNode::create(0, groundY + 40, 0, 40, 40, 110, 30);
  cyl->setMaterial(Material::fromColor(GeomColor(60, 200, 90, 255)));
  cyl->getMaterial()->roughness = 0.2f;
  cyl->getMaterial()->reflectivity = 0.4f;
  scene.addNode(cyl);

  gui << (HSplit()
          << Canvas3D(Size(1600, 1200), {.handle="canvas", .minSize={32, 24}})
          << Prop(&scene, {.label="default scene", .minSize={16, 24}}))
      << Show();
  gui["canvas"].link(scene.getGLCallback(0).get());
  gui["canvas"].install(scene.getMouseHandler(0));
}

void run() {
  gui["canvas"].render();
}

int main(int argc, char **argv) {
  return ICLApp(argc, argv, "", init, run).exec();
}
