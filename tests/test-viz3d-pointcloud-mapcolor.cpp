// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// PointCloud::mapColorFromCamera — cross-camera color→depth registration (the
// viz3d replacement for PointCloudCreator::mapImage, and the engine behind the
// stereo RGB-D simulator). GL-free (BVH capture), so verifiable headless.

#include "harness/Test.h"
#include <icl/viz3d/scene/Scene.h>
#include <icl/viz3d/render/SceneCapture.h>
#include <icl/viz3d/nodes/SphereNode.h>
#include <icl/viz3d/nodes/CuboidNode.h>
#include <icl/viz3d/pointcloud/PointCloud.h>
#include <icl/viz3d/render/Material.h>
#include <icl/cv3d/Camera.h>
#include <cmath>

using namespace icl;
using namespace icl::viz3d;
using icl::cv3d::Camera;
using icl::viz3d::Material;
using icl::cv3d::GeomColor;
using icl::utils::Size;

namespace {
  // Two distinctly-coloured shapes straddling the optical axis + a depth camera
  // (cam0) and a second camera (cam1) seeded equal to it (overwritten per test).
  void makeScene(Scene &scene, const Size &res) {
    Camera cam = Camera::lookAt(viz3d::Vec(0, -600, 350, 1), viz3d::Vec(0, 0, 40, 1),
                                viz3d::Vec(0, 0, 1, 1), res, 42.0f);
    scene.addCamera(cam);   // 0 = depth
    scene.addCamera(cam);   // 1 = color
    auto s = SphereNode::create(-90, 0, 50, 55);
    s->setMaterial(Material::fromColor(GeomColor(230, 50, 50, 255)));
    scene.addNode(s);
    auto c = CuboidNode::create(90, 0, 50, 90, 90, 100);
    c->setMaterial(Material::fromColor(GeomColor(50, 210, 80, 255)));
    scene.addNode(c);
  }

  bool isOrigin(const float *p) { return p[0]==0.f && p[1]==0.f && p[2]==0.f; }
}

// Zero baseline (cam1 == cam0): the mapped colour equals the directly-aligned
// colour for the overwhelming majority of foreground points (projecting an
// unprojected pixel back into the SAME camera returns that pixel). Background
// pixels stay unmapped (alpha 0).
ICL_REGISTER_TEST("viz3d.mapcolor.zero_baseline_matches_aligned",
                  "mapColorFromCamera with zero baseline reproduces aligned colours")
{
  const Size res(80, 60);
  Scene scene; makeScene(scene, res);
  Camera &cam0 = scene.getCamera(0);

  BVHSceneCapture cap;
  BVH::ImageResult r0 = cap.capture(scene, 0, BVH::DistToCamPlane);
  ICL_TEST_EQ(r0.depth.getDim(), res.width * res.height);

  // aligned reference: depth + colour from the same camera, per-pixel
  PointCloud aligned(res.width, res.height, PointCloud::XYZ | PointCloud::RGBA32f);
  aligned.unprojectDepth(r0.depth, cam0, true, &r0.image);

  // mapped: unproject depth, then register colour from cam1 (== cam0)
  scene.getCamera(1) = cam0;
  PointCloud mapped(res.width, res.height, PointCloud::XYZ | PointCloud::RGBA32f);
  mapped.unprojectDepth(r0.depth, cam0, true);
  mapped.mapColorFromCamera(r0.image, scene.getCamera(1));

  auto axyz = aligned.selectXYZ();   auto argb = aligned.selectRGBA32f();
  auto mxyz = mapped.selectXYZ();    auto mrgb = mapped.selectRGBA32f();

  int fg = 0, agree = 0, bgUnmapped = 0, bg = 0;
  for (int i = 0; i < res.width * res.height; ++i) {
    if (isOrigin(&axyz[i][0])) {                 // background pixel
      ++bg;
      if (mrgb[i][3] == 0.f) ++bgUnmapped;
      continue;
    }
    ++fg;
    const bool sameColor = std::fabs(argb[i][0]-mrgb[i][0]) <= 1.f &&
                           std::fabs(argb[i][1]-mrgb[i][1]) <= 1.f &&
                           std::fabs(argb[i][2]-mrgb[i][2]) <= 1.f &&
                           mrgb[i][3] == 255.f;
    if (sameColor) ++agree;
  }
  ICL_TEST_EQ(fg > 100, true);                   // the shapes are actually hit
  ICL_TEST_EQ(bg == bgUnmapped, true);           // background never coloured
  ICL_TEST_EQ(agree > fg * 0.95, true);          // mapped == aligned (≥95%)
}

// A real baseline shifts the colour camera, so foreground points still get
// coloured (registration works), but some points seen by the depth camera fall
// outside the colour camera's frustum and are left unmapped (alpha 0). This is
// the behaviour that makes the simulator a genuine RGB-D-mapping test bed.
ICL_REGISTER_TEST("viz3d.mapcolor.baseline_drops_unseen",
                  "a stereo baseline leaves some depth points uncoloured")
{
  const Size res(80, 60);
  Scene scene; makeScene(scene, res);
  Camera &cam0 = scene.getCamera(0);

  BVHSceneCapture cap;
  BVH::ImageResult r0 = cap.capture(scene, 0, BVH::DistToCamPlane);

  // large horizontal baseline so the colour view differs noticeably
  Camera colorCam = cam0;
  viz3d::Vec h = cam0.getHoriz();
  const float hn = std::sqrt(h[0]*h[0] + h[1]*h[1] + h[2]*h[2]);
  const float b = 250.f;
  colorCam.translate(viz3d::Vec(h[0]/hn*b, h[1]/hn*b, h[2]/hn*b, 0));
  scene.getCamera(1) = colorCam;
  BVH::ImageResult c1 = cap.capture(scene, 1, BVH::NoDepth);

  PointCloud cloud(res.width, res.height, PointCloud::XYZ | PointCloud::RGBA32f);
  cloud.unprojectDepth(r0.depth, cam0, true);
  cloud.mapColorFromCamera(c1.image, colorCam);

  auto xyz = cloud.selectXYZ();  auto rgb = cloud.selectRGBA32f();
  int fg = 0, mapped = 0, unmapped = 0;
  for (int i = 0; i < res.width * res.height; ++i) {
    if (isOrigin(&xyz[i][0])) continue;
    ++fg;
    if (rgb[i][3] == 255.f) ++mapped; else ++unmapped;
  }
  ICL_TEST_EQ(fg > 100, true);
  ICL_TEST_EQ(mapped > 0, true);      // registration colours real points
  ICL_TEST_EQ(unmapped > 0, true);    // ... and some depth points are unseen by colour cam
}
