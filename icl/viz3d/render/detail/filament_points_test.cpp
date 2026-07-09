// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Headless verification of the Filament backend's POINT rendering (the
// billboard-quad path with a per-vertex COLOR bind). Two sources:
//   1. MeshNode vertices with PrimVertex visible (per-vertex colours)
//   2. a PointCloudNode (dynamic cloud, RGBA32f colours)
// Renders each through Scene::renderToImage (offscreen → no GL context, runs
// in-sandbox), dumps a PNG to builddir/calib/ for eyeballing, and asserts that
// a plausible number of coloured pixels landed near the projected centroid.

#include <icl/viz3d/scene/Scene.h>
#include <icl/viz3d/nodes/MeshNode.h>
#include <icl/viz3d/nodes/PointCloudNode.h>
#include <icl/viz3d/pointcloud/PointCloud.h>
#include <icl/viz3d/render/Material.h>
#include <icl/viz3d/render/BVH.h>
#include <icl/cv3d/Camera.h>
#include <icl/core/Img.h>
#include <icl/io/SaveLoad.h>

#include <cmath>
#include <cstdio>
#include <memory>
#include <vector>

using namespace icl;

namespace {
  cv3d::Camera makeCam(int W, int H) {
    cv3d::Camera::RenderParams rp;
    rp.chipSize = {W, H};
    rp.clipZNear = 1.0f;
    rp.clipZFar = 10000.0f;
    cv3d::Camera cam;
    cam.setRenderParams(rp);
    cam.setUp(math::Vec4(0, 1, 0, 1), true);
    return cam;
  }

  // Count coloured (non-background) pixels + their centroid.
  long litPixels(const core::Img8u &img, float &cx, float &cy) {
    const int W = img.getWidth(), H = img.getHeight();
    core::Channel8u r = img[0], g = img[1], b = img[2];
    long lit = 0; double sx = 0, sy = 0;
    for (int y = 0; y < H; ++y)
      for (int x = 0; x < W; ++x)
        if ((r(x, y) + g(x, y) + b(x, y)) / 3 > 20) { ++lit; sx += x; sy += y; }
    cx = lit ? sx / lit : -1; cy = lit ? sy / lit : -1;
    return lit;
  }

  void save(const core::Img8u &img, const char *path) {
    try { io::save(core::Image(img), path); }
    catch (...) { std::fprintf(stderr, "  (could not save %s)\n", path); }
  }
}

int main() {
  const int W = 640, H = 480;
  cv3d::Camera cam = makeCam(W, H);
  int failures = 0;

  // --- 1) MeshNode vertices as points ---------------------------------------
  {
    viz3d::Scene scene;
    scene.addCamera(cam);
    auto mesh = std::make_shared<viz3d::MeshNode>();
    // 5x5 grid of points in the z=0 plane, spread ±100 units, rainbow-ish.
    for (int j = 0; j < 5; ++j)
      for (int i = 0; i < 5; ++i) {
        float x = (i - 2) * 50.f, y = (j - 2) * 50.f;
        viz3d::GeomColor c(255.f * i / 4, 255.f * j / 4, 255.f * (8 - i - j) / 8, 255.f);
        mesh->addVertex(math::Vec4(x, y, 0, 1), c);
      }
    mesh->setPointSize(10.f);
    scene.addNode(mesh);

    // Look at the grid from +Z.
    cam.setPosition(math::Vec4(0, 0, 600, 1));
    cam.setUp(math::Vec4(0, 1, 0, 1), true);
    scene.getCamera(0) = cam;

    auto res = scene.renderToImage(0, viz3d::BVH::NoDepth);
    float cx, cy; long lit = litPixels(res.image, cx, cy);
    save(res.image, "calib/points-mesh.png");
    utils::Point32f ctr = cam.project(math::Vec4(0, 0, 0, 1));
    float err = std::sqrt((cx - ctr.x) * (cx - ctr.x) + (cy - ctr.y) * (cy - ctr.y));
    std::printf("mesh-points: %ld lit px, centroid=(%.1f,%.1f) vs project=(%.1f,%.1f) err=%.1f\n",
                lit, cx, cy, ctr.x, ctr.y, err);
    // 25 points * ~10px squares ≈ a couple thousand px; centroid ≈ image centre.
    if (lit < 500 || err > 30.f) { std::fprintf(stderr, "  FAIL mesh points\n"); ++failures; }
  }

  // --- 2) PointCloudNode ----------------------------------------------------
  {
    viz3d::Scene scene;
    scene.addCamera(cam);
    const int N = 400;
    auto cloud = std::make_shared<viz3d::PointCloud>(N);
    cloud->addFeature(viz3d::PointCloud::RGBA32f);
    {
      auto xyz = cloud->selectXYZ();
      auto rgba = cloud->selectRGBA32f();
      for (int k = 0; k < N; ++k) {
        float a = k * 0.35f, rr = 40.f + 0.4f * k;   // spiral in z=0 plane
        xyz[k][0] = rr * std::cos(a); xyz[k][1] = rr * std::sin(a); xyz[k][2] = 0;
        rgba[k][0] = 255.f * (k % 5) / 4; rgba[k][1] = 255.f * (k % 7) / 6;
        rgba[k][2] = 255.f * (k % 3) / 2; rgba[k][3] = 255.f;
      }
    }
    auto pcn = std::make_shared<viz3d::PointCloudNode>(cloud);
    pcn->setPointSize(6.f);
    scene.addNode(pcn);

    cam.setPosition(math::Vec4(0, 0, 700, 1));
    cam.setUp(math::Vec4(0, 1, 0, 1), true);
    scene.getCamera(0) = cam;

    auto res = scene.renderToImage(0, viz3d::BVH::NoDepth);
    float cx, cy; long lit = litPixels(res.image, cx, cy);
    save(res.image, "calib/points-cloud.png");
    std::printf("cloud-points: %ld lit px, centroid=(%.1f,%.1f)\n", lit, cx, cy);
    if (lit < 500) { std::fprintf(stderr, "  FAIL cloud points\n"); ++failures; }
  }

  if (failures) { std::fprintf(stderr, "points-test: FAIL (%d)\n", failures); return 1; }
  std::printf("points-test: PASS\n");
  return 0;
}
