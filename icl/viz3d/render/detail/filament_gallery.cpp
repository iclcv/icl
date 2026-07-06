// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Headless colour/material-tuning harness: builds the DefaultScene(Studio) with a
// few coloured shapes, renders it through the (default) Filament backend via
// Scene::renderToImage — no GL context — and writes a PNG. Lets the look be
// eyeballed + tuned in-sandbox against the GL reference, without a display.
// Usage: icl-filament-gallery [out.png]

#include <icl/viz3d/scene/DefaultScene.h>
#include <icl/viz3d/nodes/SphereNode.h>
#include <icl/viz3d/nodes/CuboidNode.h>
#include <icl/viz3d/nodes/CylinderNode.h>
#include <icl/viz3d/nodes/ConeNode.h>
#include <icl/viz3d/render/Material.h>
#include <icl/viz3d/render/BVH.h>
#include <icl/cv3d/Camera.h>
#include <icl/io/SaveLoad.h>

#include <cstdio>

using namespace icl;
using viz3d::GeomColor;
using viz3d::Material;

int main(int argc, char **argv) {
  viz3d::DefaultScene scene(viz3d::DefaultScene::SceneType::Studio);
  const float g = -208.0f;   // ground level (Studio default extent 400)

  auto sphere = viz3d::SphereNode::create(120, g + 60, 0, 60, 40, 40);
  sphere->setMaterial(Material::fromColor(GeomColor(220, 60, 60, 255)));
  scene.addNode(sphere);

  auto cube = viz3d::CuboidNode::create(-120, g + 45, 0, 90, 90, 90);
  cube->setMaterial(Material::fromColor(GeomColor(60, 90, 220, 255)));
  scene.addNode(cube);

  auto cyl = viz3d::CylinderNode::create(0, g + 40, 0, 40, 40, 110, 30);
  cyl->setMaterial(Material::fromColor(GeomColor(60, 200, 90, 255)));
  scene.addNode(cyl);

  auto cone = viz3d::ConeNode::create(220, g, 0, 60, 120, 30);
  cone->setMaterial(Material::fromColor(GeomColor(230, 210, 60, 255)));
  scene.addNode(cone);

  if (scene.getCameraCount() == 0) { std::fprintf(stderr, "no camera\n"); return 2; }
  scene.getCamera(0).getRenderParams().chipSize = utils::Size(800, 600);

  // Optional 2nd arg "unlit" → toggle the scene's "enable lighting" property off.
  if (argc > 2 && std::string(argv[2]) == "unlit")
    scene.setPropertyValue("enable lighting", false);

  viz3d::BVH::ImageResult res = scene.renderToImage(0, viz3d::BVH::NoDepth);
  if (res.image.getDim() == 0) { std::fprintf(stderr, "empty render\n"); return 1; }

  const std::string out = argc > 1 ? argv[1] : "/tmp/filament_gallery.png";
  io::save(core::Image(res.image), out);
  std::printf("wrote %s (%dx%d)\n", out.c_str(), res.image.getWidth(), res.image.getHeight());
  return 0;
}
