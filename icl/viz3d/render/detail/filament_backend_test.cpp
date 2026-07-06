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

  // A light-grey cube at the origin, edge 3, rotated so three faces show with
  // different normals → different shading (proves lit PBR + normals + lights).
  auto cube = std::make_shared<viz3d::CuboidNode>(0.f, 0.f, 0.f, 3.f);
  cube->rotate(0.5f, 0.7f, 0.0f);
  auto mat = std::make_shared<viz3d::Material>();
  mat->baseColor = {0.8f, 0.8f, 0.8f, 1.0f};
  mat->roughness = 0.5f;
  cube->setMaterial(mat);
  std::vector<std::shared_ptr<viz3d::Node>> nodes{cube};

  backend.render(nodes, cam.getCSTransformationMatrixGL(), cam.getProjectionMatrixGL());

  core::Img8u img;
  if (!backend.readColor(img)) {
    std::fprintf(stderr, "backend-test: readColor failed\n");
    return 3;
  }

  // Lit pixels = non-background (background is black). Count + centroid + the
  // per-pixel brightness distribution over the cube.
  core::Channel8u r = img[0], g = img[1], b = img[2];
  long n = 0; double sx = 0, sy = 0, sB = 0, sB2 = 0;
  int bMin = 255, bMax = 0;
  for (int y = 0; y < H; ++y)
    for (int x = 0; x < W; ++x) {
      int lum = (r(x, y) + g(x, y) + b(x, y)) / 3;
      if (lum > 20) {   // above background
        ++n; sx += x; sy += y; sB += lum; sB2 += double(lum) * lum;
        bMin = std::min(bMin, lum); bMax = std::max(bMax, lum);
      }
    }

  utils::Point32f projCentre = cam.project(math::Vec4(0, 0, 0, 1));
  double meanB = n ? sB / n : 0;
  double stdB = n ? std::sqrt(std::max(0.0, sB2 / n - meanB * meanB)) : 0;
  std::printf("backend-test: %ld lit px, centroid=(%.1f,%.1f) vs project=(%.1f,%.1f); "
              "lum mean=%.0f std=%.0f range=[%d,%d]\n",
              n, n ? sx / n : -1, n ? sy / n : -1, projCentre.x, projCentre.y,
              meanB, stdB, bMin, bMax);

  if (n < 5000) {
    std::fprintf(stderr, "backend-test: FAIL — too few cube pixels (%ld)\n", n);
    return 1;
  }
  float cx = sx / n, cy = sy / n;
  float err = std::sqrt((cx - projCentre.x) * (cx - projCentre.x) +
                        (cy - projCentre.y) * (cy - projCentre.y));
  if (err > 8.0f) {   // rotated cube centroid is near (not exactly on) the centre
    std::fprintf(stderr, "backend-test: FAIL — centroid off by %.1fpx\n", err);
    return 1;
  }
  // Faces at different angles must differ in brightness (lit shading, not flat fill).
  if (bMax - bMin < 30) {
    std::fprintf(stderr, "backend-test: FAIL — no shading gradient (range %d)\n", bMax - bMin);
    return 1;
  }
  std::printf("backend-test: PASS — lit cube, faces shaded (range %d), centroid %.1fpx off\n",
              bMax - bMin, err);
  return 0;
}
