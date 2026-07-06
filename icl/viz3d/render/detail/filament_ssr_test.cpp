// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// SSR check: a glossy floor + a bright cube, driven through the REAL backend over
// several frames (Filament SSR reflects the reprojected previous frame, so it
// needs history — a single renderToImage can't show it, which is why this uses
// the backend directly rather than Scene::renderToImage). Writes a PNG to eyeball
// whether the floor reflects the cube. Optional arg "off" disables SSR to compare.

#include <icl/viz3d/render/detail/FilamentRenderBackend.h>
#include <icl/viz3d/nodes/CuboidNode.h>
#include <icl/viz3d/nodes/LightNode.h>
#include <icl/viz3d/render/Material.h>
#include <icl/cv3d/Camera.h>
#include <icl/core/Img.h>
#include <icl/io/SaveLoad.h>

#include <cstdio>
#include <memory>
#include <vector>

using namespace icl;

int main(int argc, char **argv) {
  viz3d::FilamentRenderBackend backend;
  if (!backend.isValid()) { std::fprintf(stderr, "no engine\n"); return 2; }
  const bool ssrOff = argc > 2 && std::string(argv[2]) == "off";
  backend.setSSREnabled(!ssrOff);

  const int W = 800, H = 600;
  backend.setTargetSize({W, H});

  cv3d::Camera::RenderParams rp;
  rp.chipSize = {W, H};
  rp.clipZNear = 0.5f; rp.clipZFar = 5000.0f;
  cv3d::Camera cam;
  cam.setRenderParams(rp);
  cam.setPosition(math::Vec4(0, 6, 16, 1));
  cam.setNorm(math::Vec4(0, -0.35f, -1, 1), true);   // look down toward the floor
  cam.setUp(math::Vec4(0, 1, 0, 1), true);

  // Glossy near-mirror floor (roughness low) + a bright red cube above it.
  auto floor = std::make_shared<viz3d::CuboidNode>(0.f, -0.25f, 0.f, 40.f, 0.5f, 40.f);
  auto fmat = std::make_shared<viz3d::Material>();
  fmat->baseColor = {0.6f, 0.6f, 0.62f, 1.0f};
  fmat->roughness = 0.05f;   // near-mirror → SSR should show
  floor->setMaterial(fmat);

  auto cube = std::make_shared<viz3d::CuboidNode>(0.f, 3.f, 0.f, 3.f);
  auto cmat = std::make_shared<viz3d::Material>();
  cmat->baseColor = {1.0f, 0.1f, 0.1f, 1.0f};   // bright red — easy to spot in a reflection
  cube->setMaterial(cmat);

  auto light = viz3d::LightNode::directional(-0.3f, -1.0f, -0.4f);
  std::vector<std::shared_ptr<viz3d::Node>> nodes{floor, cube, light};

  // Several frames so Filament's screen-space reflection history builds up.
  core::Img8u img;
  for (int i = 0; i < 5; ++i) {
    backend.render(nodes, cam.getCSTransformationMatrixGL(), cam.getProjectionMatrixGL());
    backend.readColor(img);
  }

  const char *out = argc > 1 ? argv[1] : "/tmp/filament_ssr.png";
  io::save(core::Image(img), out);

  // Metric: red-dominant pixels in the UPPER floor band (the receding floor above
  // the cube, where its reflection lands — NOT the cube itself, which is centred
  // lower). SSR on → many (the reflection); SSR off → ~0.
  core::Channel8u r = img[0], g = img[1], b = img[2];
  long reflRed = 0;
  for (int y = 40; y < H / 3; ++y)
    for (int x = 0; x < W; ++x)
      if (r(x, y) > g(x, y) + 25 && r(x, y) > b(x, y) + 25) ++reflRed;
  std::printf("ssr-test(%s): wrote %s; reflected-red px = %ld\n",
              ssrOff ? "OFF" : "ON", out, reflRed);
  return 0;
}
