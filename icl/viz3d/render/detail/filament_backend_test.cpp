// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// P2 FilamentRenderBackend headless driver test. Builds a small ICL scene graph
// (a coloured cuboid), renders it through the REAL FilamentRenderBackend (not the
// raw Filament API), reads the colour image back, and asserts:
//   1. the cuboid renders (a healthy fraction of pixels carry its base colour), and
//   2. the lit region is centred on cam.project(cuboid centre) — i.e. geometry
//      translation + the projection recipe agree end-to-end, through the class.
// Runs headless in-sandbox (Metal).

#include <icl/viz3d/render/detail/FilamentRenderBackend.h>
#include <icl/viz3d/nodes/CuboidNode.h>
#include <icl/viz3d/render/Material.h>
#include <icl/cv3d/Camera.h>
#include <icl/core/Img.h>

#include <cmath>
#include <cstdio>
#include <memory>
#include <vector>

using namespace icl;

int main() {
  viz3d::FilamentRenderBackend backend;
  if (!backend.isValid()) {
    std::fprintf(stderr, "backend-test: Filament engine unavailable\n");
    return 2;
  }

  const int W = 640, H = 480;
  backend.setTargetSize({W, H});

  // Camera with a real off-centre principal point, looking down -z at the origin.
  cv3d::Camera::RenderParams rp;
  rp.chipSize = {W, H};
  rp.clipZNear = 1.0f;
  rp.clipZFar = 10000.0f;
  cv3d::Camera cam;
  cam.setRenderParams(rp);
  cam.setPrincipalPointOffset(340.0f, 250.0f);

  // A red cube at the origin, edge 3 (world units), camera at (0,0,10).
  auto cube = std::make_shared<viz3d::CuboidNode>(0.f, 0.f, 0.f, 3.f);
  auto mat = std::make_shared<viz3d::Material>();
  mat->baseColor = {1.0f, 0.0f, 0.0f, 1.0f};
  cube->setMaterial(mat);
  std::vector<std::shared_ptr<viz3d::Node>> nodes{cube};

  backend.render(nodes, cam.getCSTransformationMatrixGL(), cam.getProjectionMatrixGL());

  core::Img8u img;
  if (!backend.readColor(img)) {
    std::fprintf(stderr, "backend-test: readColor failed\n");
    return 3;
  }

  // Find red pixels (the cube), compute count + centroid.
  core::Channel8u r = img[0], g = img[1], b = img[2];
  long n = 0; double sx = 0, sy = 0;
  for (int y = 0; y < H; ++y)
    for (int x = 0; x < W; ++x)
      if (r(x, y) > 100 && g(x, y) < 80 && b(x, y) < 80) { ++n; sx += x; sy += y; }

  utils::Point32f projCentre = cam.project(math::Vec4(0, 0, 0, 1));
  std::printf("backend-test: %ld red px, centroid=(%.1f,%.1f), cam.project(centre)=(%.1f,%.1f)\n",
              n, n ? sx / n : -1, n ? sy / n : -1, projCentre.x, projCentre.y);

  if (n < 500) {   // a 3-unit cube at depth 10 with f=600 → ~180px box → ~30k px
    std::fprintf(stderr, "backend-test: FAIL — too few cube pixels (%ld)\n", n);
    return 1;
  }
  float cx = sx / n, cy = sy / n;
  float err = std::sqrt((cx - projCentre.x) * (cx - projCentre.x) +
                        (cy - projCentre.y) * (cy - projCentre.y));
  if (err > 3.0f) {   // centroid of a symmetric cube face ≈ projected centre
    std::fprintf(stderr, "backend-test: FAIL — centroid off by %.1fpx\n", err);
    return 1;
  }
  std::printf("backend-test: PASS — cube renders, centroid within %.1fpx of projection\n", err);
  return 0;
}
