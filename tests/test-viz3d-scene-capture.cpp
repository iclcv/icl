// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// SceneCapture (CPU / BVH backend) tests — GL-free, so verifiable headless in
// the sandbox. The GL backend (GLSceneCapture / Scene2::renderToImage) shares
// this exact ImageResult/DepthMode contract but needs a real GL context, so it
// is exercised only on a real display.

#include "harness/Test.h"
#include <icl/viz3d/Scene2.h>
#include <icl/viz3d/SceneCapture.h>
#include <icl/viz3d/CuboidNode.h>
#include <icl/viz3d/Material.h>
#include <icl/cv3d/Camera.h>
#include <cmath>
#include <memory>

using namespace icl;
using namespace icl::viz3d;
using icl::geom::Camera;
using icl::viz3d::Material;
using icl::geom::GeomColor;
using icl::utils::Size;

namespace {
  // A single 200mm cube centred at the origin, viewed head-on from +Z at 600mm.
  // The front face sits at z=+100 → plane-depth at the optical centre ≈ 500mm.
  // (Scene2 is non-movable, so we populate a caller-owned instance.)
  void makeCubeScene(Scene2 &scene, const Size &res) {
    scene.addCamera(Camera::lookAt(viz3d::Vec(0, 0, 600, 1),   // eye
                                   viz3d::Vec(0, 0, 0, 1),     // center
                                   viz3d::Vec(0, 1, 0, 1),     // up
                                   res, 40.0f));
    auto cube = CuboidNode::create(0, 0, 0, 200, 200, 200);
    cube->setMaterial(Material::fromColor(GeomColor(200, 60, 60, 255)));
    scene.addNode(cube);
  }
}

// The CPU backend fills both buffers at the camera resolution, the cube is hit
// somewhere (non-background color + positive depth), and the centre pixel lands
// on the front face at ~500mm.
ICL_REGISTER_TEST("viz3d.scenecapture.bvh_color_and_depth", "BVH capture yields sized color+depth, cube hit at ~500mm")
{
  const Size res(64, 48);
  Scene2 scene; makeCubeScene(scene, res);

  BVHSceneCapture cap;
  BVH::ImageResult r = cap.capture(scene, 0, BVH::DistToCamPlane);

  ICL_TEST_EQ(r.image.getSize() == res, true);
  ICL_TEST_EQ(r.image.getChannels(), 3);
  ICL_TEST_EQ(r.depth.getSize() == res, true);
  ICL_TEST_EQ(r.depth.getChannels(), 1);

  // Centre pixel hits the front face: red, ~500mm.
  const int cx = res.width / 2, cy = res.height / 2;
  const int ci = cy * res.width + cx;
  const float dCenter = r.depth.getData(0)[ci];
  ICL_TEST_EQ(dCenter > 450.f && dCenter < 550.f, true);
  ICL_TEST_EQ(r.image.getData(0)[ci] > 100, true);   // red channel lit

  // At least some pixels are background (the cube does not fill the frame).
  bool anyBackground = false;
  for (int i = 0; i < res.width * res.height; ++i)
    if (r.depth.getData(0)[i] == 0.f) { anyBackground = true; break; }
  ICL_TEST_EQ(anyBackground, true);
}

// Invalid camera index → empty result (no throw, no out-of-range read).
ICL_REGISTER_TEST("viz3d.scenecapture.bvh_invalid_camera", "BVH capture returns empty on bad camera index")
{
  Scene2 scene; makeCubeScene(scene, Size(32, 24));
  BVHSceneCapture cap;
  BVH::ImageResult r = cap.capture(scene, 7, BVH::DistToCamPlane);
  ICL_TEST_EQ(r.image.getDim(), 0);
  ICL_TEST_EQ(r.depth.getDim(), 0);
}

// The two depth conventions agree at the optical centre (the centre view ray is
// ~parallel to the viewing axis) but DistToCamCenter (Euclidean) exceeds
// DistToCamPlane (Z-depth) toward the frame edges, where the rays slant.
ICL_REGISTER_TEST("viz3d.scenecapture.depth_modes", "DistToCamCenter >= DistToCamPlane, equal at centre")
{
  const Size res(80, 60);
  Scene2 scene; makeCubeScene(scene, res);

  BVHSceneCapture cap;
  BVH::ImageResult plane  = cap.capture(scene, 0, BVH::DistToCamPlane);
  BVH::ImageResult center = cap.capture(scene, 0, BVH::DistToCamCenter);

  const int cx = res.width / 2, cy = res.height / 2;
  const int ci = cy * res.width + cx;
  // Centre: nearly identical (within 1mm).
  ICL_TEST_EQ(std::fabs(center.depth.getData(0)[ci] - plane.depth.getData(0)[ci]) < 1.0f, true);

  // Find a hit pixel away from the centre and confirm Euclidean > Z-depth there.
  bool foundSlanted = false;
  const float *dp = plane.depth.getData(0), *dc = center.depth.getData(0);
  for (int y = 0; y < res.height && !foundSlanted; ++y) {
    for (int x = 0; x < res.width; ++x) {
      const int i = y * res.width + x;
      if (dp[i] > 0.f && (std::abs(x - cx) > res.width / 4)) {
        ICL_TEST_EQ(dc[i] > dp[i], true);
        foundSlanted = true;
        break;
      }
    }
  }
  ICL_TEST_EQ(foundSlanted, true);
}

// Caching reuses the BVH (same result as uncached) and only reflects geometry
// changes after invalidate() — guards the per-frame-rebuild optimization.
ICL_REGISTER_TEST("viz3d.scenecapture.bvh_caching", "cached capture matches uncached; invalidate picks up changes")
{
  const Size res(48, 36);
  Scene2 scene; makeCubeScene(scene, res);
  const int ci = (res.height / 2) * res.width + res.width / 2;

  BVHSceneCapture cached;   cached.setCaching(true);
  BVHSceneCapture uncached;
  const float dCached   = cached.capture(scene, 0, BVH::DistToCamPlane).depth.getData(0)[ci];
  const float dUncached = uncached.capture(scene, 0, BVH::DistToCamPlane).depth.getData(0)[ci];
  ICL_TEST_EQ(std::fabs(dCached - dUncached) < 1e-3f, true);   // identical geometry

  // Add a nearer cube covering the centre. The cache is stale → unchanged depth…
  auto near = CuboidNode::create(0, 0, 400, 300, 300, 50);   // front face at z≈425
  near->setMaterial(Material::fromColor(GeomColor(60, 220, 60, 255)));
  scene.addNode(near);
  const float dStale = cached.capture(scene, 0, BVH::DistToCamPlane).depth.getData(0)[ci];
  ICL_TEST_EQ(std::fabs(dStale - dCached) < 1e-3f, true);

  // …until invalidate(), after which the nearer surface shows (smaller depth).
  cached.invalidate();
  const float dFresh = cached.capture(scene, 0, BVH::DistToCamPlane).depth.getData(0)[ci];
  ICL_TEST_EQ(dFresh < dStale - 50.f, true);
}
