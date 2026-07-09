// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Headless verification of the Filament backend's GLASS / screen-space refraction
// path (Material::TransmissionParams → glass_pbr.mat). A transmissive sphere sits
// in front of a coloured opaque backdrop; with transmission the backdrop shows
// (refracted) through the sphere instead of the sphere painting a flat colour.
// Renders through Scene::renderToImage (offscreen, no GL context → runs
// in-sandbox), dumps a PNG for eyeballing, and asserts the sphere footprint is
// NOT a flat opaque disc (i.e. background bleeds through).

#include <icl/viz3d/scene/Scene.h>
#include <icl/viz3d/nodes/SphereNode.h>
#include <icl/viz3d/nodes/CuboidNode.h>
#include <icl/viz3d/nodes/LightNode.h>
#include <icl/viz3d/render/Material.h>
#include <icl/viz3d/render/BVH.h>
#include <icl/cv3d/Camera.h>
#include <icl/core/Img.h>
#include <icl/io/SaveLoad.h>

#include <cstdio>
#include <memory>

using namespace icl;

int main() {
  const int W = 640, H = 480;
  cv3d::Camera::RenderParams rp;
  rp.chipSize = {W, H};
  rp.clipZNear = 1.0f;
  rp.clipZFar = 10000.0f;
  cv3d::Camera cam;
  cam.setRenderParams(rp);
  cam.setPosition(math::Vec4(0, 0, 800, 1));
  cam.setUp(math::Vec4(0, 1, 0, 1), true);

  viz3d::Scene scene;
  scene.addCamera(cam);

  // Green backdrop wall behind everything.
  auto wall = std::make_shared<viz3d::CuboidNode>(0.f, 0.f, -200.f, 800.f, 800.f, 10.f);
  auto wmat = std::make_shared<viz3d::Material>();
  wmat->baseColor = {0.1f, 0.8f, 0.2f, 1.0f};
  wmat->roughness = 0.8f;
  wall->setMaterial(wmat);
  scene.addNode(wall);

  // A small red cube between the wall and the sphere — a landmark to see refracted.
  auto cube = std::make_shared<viz3d::CuboidNode>(0.f, 0.f, -60.f, 90.f);
  auto cmat = std::make_shared<viz3d::Material>();
  cmat->baseColor = {0.9f, 0.1f, 0.1f, 1.0f};
  cube->setMaterial(cmat);
  scene.addNode(cube);

  // Glass sphere in front.
  auto sphere = std::make_shared<viz3d::SphereNode>(0.f, 0.f, 120.f, 130.f);
  auto gmat = std::make_shared<viz3d::Material>();
  gmat->baseColor = {1.0f, 1.0f, 1.0f, 1.0f};
  gmat->roughness = 0.05f;
  gmat->metallic = 0.0f;
  gmat->transmission = std::make_shared<viz3d::Material::TransmissionParams>();
  gmat->transmission->transmission = 1.0f;
  gmat->transmission->ior = 1.5f;
  sphere->setMaterial(gmat);
  scene.addNode(sphere);

  scene.addNode(viz3d::LightNode::point(300, 400, 600));

  auto res = scene.renderToImage(0, viz3d::BVH::NoDepth);
  core::Img8u &img = res.image;
  try { io::save(core::Image(img), "calib/glass.png"); }
  catch (...) { std::fprintf(stderr, "  (could not save calib/glass.png)\n"); }

  // Sample the sphere centre region: it should contain reddish/greenish
  // background bleed-through, not a flat white/grey disc. Count distinct-ish hue
  // pixels (red or green dominant) inside the central sphere footprint.
  core::Channel8u r = img[0], g = img[1], b = img[2];
  long redish = 0, greenish = 0, total = 0;
  for (int y = H / 2 - 40; y < H / 2 + 40; ++y)
    for (int x = W / 2 - 40; x < W / 2 + 40; ++x) {
      int R = r(x, y), G = g(x, y), B = b(x, y);
      ++total;
      if (R > G + 25 && R > B + 25) ++redish;
      if (G > R + 20 && G > B + 10) ++greenish;
    }
  std::printf("glass: centre patch %ld px — redish=%ld greenish=%ld\n", total, redish, greenish);
  if (redish + greenish < 100) {
    std::fprintf(stderr, "glass: FAIL — no background bleed-through (refraction inert?)\n");
    return 1;
  }
  std::printf("glass: PASS — background refracts through the transmissive sphere\n");
  return 0;
}
