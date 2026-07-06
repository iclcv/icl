// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Scene-level Filament wiring test. Constructs a real viz3d::Scene (which now
// defaults to the Filament backend when available), adds a camera + a coloured
// cube, and calls Scene::renderToImage — the offscreen path, which needs no GL
// context and so runs headless in-sandbox. Proves makeRenderBackend() selects
// Filament and Scene::renderToImage routes through it end-to-end.

#include <icl/viz3d/scene/Scene.h>
#include <icl/viz3d/nodes/CuboidNode.h>
#include <icl/viz3d/render/Material.h>
#include <icl/viz3d/render/BVH.h>
#include <icl/cv3d/Camera.h>
#include <icl/core/Img.h>

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
  cam.setPrincipalPointOffset(340.0f, 250.0f);
  cam.setUp(math::Vec4(0, 1, 0, 1), true);

  viz3d::Scene scene;
  scene.addCamera(cam);

  auto cube = std::make_shared<viz3d::CuboidNode>(0.f, 0.f, 0.f, 3.f);
  auto mat = std::make_shared<viz3d::Material>();
  mat->baseColor = {0.8f, 0.2f, 0.2f, 1.0f};   // reddish
  cube->setMaterial(mat);
  scene.addNode(cube);

  viz3d::BVH::ImageResult res = scene.renderToImage(0, viz3d::BVH::NoDepth);
  core::Img8u &img = res.image;
  if (img.getWidth() != W || img.getHeight() != H) {
    std::fprintf(stderr, "scene-test: bad image size %dx%d\n", img.getWidth(), img.getHeight());
    return 1;
  }

  core::Channel8u r = img[0], g = img[1], b = img[2];
  long lit = 0; double sx = 0, sy = 0;
  for (int y = 0; y < H; ++y)
    for (int x = 0; x < W; ++x)
      if ((r(x, y) + g(x, y) + b(x, y)) / 3 > 20) { ++lit; sx += x; sy += y; }

  utils::Point32f centre = cam.project(math::Vec4(0, 0, 0, 1));
  float cx = lit ? sx / lit : -1, cy = lit ? sy / lit : -1;
  float err = lit ? std::sqrt((cx - centre.x) * (cx - centre.x) + (cy - centre.y) * (cy - centre.y)) : 1e9f;
  std::printf("scene-test: %ld lit px, centroid=(%.1f,%.1f) vs project=(%.1f,%.1f) err=%.1f\n",
              lit, cx, cy, centre.x, centre.y, err);

  if (lit < 5000 || err > 6.0f) {
    std::fprintf(stderr, "scene-test: FAIL — cube not rendered through Scene::renderToImage\n");
    return 1;
  }
  std::printf("scene-test: PASS — Scene defaults to Filament; renderToImage works headless\n");
  return 0;
}
