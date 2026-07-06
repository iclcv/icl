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
#include <icl/viz3d/nodes/MeshNode.h>
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
  cam.setUp(math::Vec4(0, 1, 0, 1), true);   // conventional y-up (default is x-up)

  // A light-grey cube at the origin, edge 3, rotated so three faces show with
  // different normals → different shading (proves lit PBR + normals + lights).
  auto cube = std::make_shared<viz3d::CuboidNode>(0.f, 0.f, 0.f, 3.f);
  cube->rotate(0.5f, 0.7f, 0.0f);
  auto mat = std::make_shared<viz3d::Material>();
  mat->baseColor = {0.8f, 0.8f, 0.8f, 1.0f};
  mat->roughness = 0.5f;
  cube->setMaterial(mat);

  // A pure-green horizontal line above the cube (tests LINES + vertex colour).
  auto mesh = std::make_shared<viz3d::MeshNode>();
  mesh->addVertex(math::Vec4(-2, 2.5f, 0, 1));
  mesh->addVertex(math::Vec4(2, 2.5f, 0, 1));
  mesh->addLine(0, 1, viz3d::GeomColor(0, 255, 0, 255));   // addLine uses 0-255

  // A bright-blue cube BEHIND the grey one (camera at z=10 → z=-4 is farther),
  // small enough to sit fully inside the grey cube's silhouette. If depth test +
  // face culling are correct it is fully occluded → no blue shows. This guards
  // against the "inside-out / hollow" bug (wrong faces culled, or no depth).
  auto back = std::make_shared<viz3d::CuboidNode>(0.f, 0.f, -4.f, 1.2f);
  auto bmat = std::make_shared<viz3d::Material>();
  bmat->baseColor = {0.0f, 0.0f, 1.0f, 1.0f};
  back->setMaterial(bmat);

  std::vector<std::shared_ptr<viz3d::Node>> nodes{cube, mesh, back};

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

  // The green line must render: pure-green pixels (g high, r/b low) that the
  // grey lit cube can't produce.
  // The green vertex-coloured line: detect "green-dominant" pixels (ACES bleeds
  // pure green, so r/b aren't near-zero) and check they cluster at the line's
  // projected midpoint.
  long green = 0; double gsx = 0, gsy = 0;
  for (int y = 0; y < H; ++y)
    for (int x = 0; x < W; ++x)
      if (g(x, y) > r(x, y) + 20 && g(x, y) > b(x, y) + 20 && g(x, y) > 40) {
        ++green; gsx += x; gsy += y;
      }
  utils::Point32f lineMid = cam.project(math::Vec4(0, 2.5f, 0, 1));
  float lerr = green ? std::sqrt((gsx / green - lineMid.x) * (gsx / green - lineMid.x) +
                                 (gsy / green - lineMid.y) * (gsy / green - lineMid.y)) : 1e9f;
  std::printf("backend-test: %ld green line px, centroid=(%.1f,%.1f) vs project=(%.1f,%.1f) err=%.1f\n",
              green, green ? gsx / green : -1, green ? gsy / green : -1,
              lineMid.x, lineMid.y, lerr);
  if (green < 20) {
    std::fprintf(stderr, "backend-test: FAIL — line did not render (%ld green px)\n", green);
    return 1;
  }
  if (lerr > 6.0f) {
    std::fprintf(stderr, "backend-test: FAIL — line mis-placed (%.1fpx off)\n", lerr);
    return 1;
  }

  // Occlusion: the blue back-cube must be hidden behind the grey one.
  long blue = 0;
  for (int y = 0; y < H; ++y)
    for (int x = 0; x < W; ++x)
      if (b(x, y) > r(x, y) + 40 && b(x, y) > g(x, y) + 40) ++blue;
  std::printf("backend-test: %ld blue (should-be-occluded) px\n", blue);
  if (blue > 100) {
    std::fprintf(stderr, "backend-test: FAIL — back cube shows through (%ld blue px): "
                         "depth test or face culling is broken\n", blue);
    return 1;
  }

  std::printf("backend-test: PASS — lit cube shaded (range %d, %.1fpx off) + line (%.1fpx) + occlusion OK\n",
              bMax - bMin, err, lerr);
  return 0;
}
