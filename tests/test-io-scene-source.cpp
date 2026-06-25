// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// The "scene" image-source backend (icl/geom2/detail/SceneSource): a synthetic
// depth/RGBD/color camera over a built-in geom2 scene. GL-free (CPU raytrace),
// so it runs headless. Tests cover the output formats, the camera-in-metadata
// round-trip, and the full depth -> camera -> point-cloud reconstruction.

#include "harness/Test.h"
#include <icl/io/source/ImageSource.h>
#include <icl/geom/Camera.h>
#include <icl/geom/PointCloudCreator.h>
#include <icl/geom/PointCloudObject.h>
#include <icl/geom2/PointCloud.h>
#include <icl/geom2/PointCloudSource.h>
#include <icl/core/Image.h>
#include <sstream>
#include <cmath>

using namespace icl;
using namespace icl::core;
using icl::io::ImageSource;
using icl::geom::Camera;
using icl::utils::Size;

// Default format is a VGA float depth image (mm) with the depth camera attached
// to the image metadata.
ICL_REGISTER_TEST("io.scenesource.depth_and_camera", "scene source yields VGA depth + camera metadata")
{
  ImageSource src("scene", "@animate=off");
  Image depth = src.grab();
  ICL_TEST_EQ(depth.getSize() == Size::VGA, true);
  ICL_TEST_EQ(depth.getChannels(), 1);
  ICL_TEST_EQ((int)depth.getDepth(), (int)depth32f);

  // The camera round-trips out of the metadata.
  ICL_TEST_EQ(depth.ptr()->hasMetaData(), true);
  std::istringstream is(depth.ptr()->getMetaData());
  Camera cam; is >> cam;
  ICL_TEST_EQ(cam.getResolution() == Size::VGA, true);

  // A good chunk of pixels hit the content/ground (positive depth).
  const Img32f &d = depth.as<icl32f>();
  int hits = 0;
  for (int i = 0; i < d.getDim(); ++i) if (d.begin(0)[i] > 1.f) ++hits;
  ICL_TEST_EQ(hits > 1000, true);
}

// format=color -> 8-bit RGB.
ICL_REGISTER_TEST("io.scenesource.color", "format=color yields an 8-bit RGB image")
{
  ImageSource src("scene", "@animate=off@format=color");
  Image c = src.grab();
  ICL_TEST_EQ(c.getSize() == Size::VGA, true);
  ICL_TEST_EQ(c.getChannels(), 3);
  ICL_TEST_EQ((int)c.getDepth(), (int)depth8u);
}

// format=rgbd -> 4-ch float: ch0-2 = RGB [0,255], ch3 = depth (mm).
ICL_REGISTER_TEST("io.scenesource.rgbd_pack", "format=rgbd packs color + depth into one 4-ch float image")
{
  ImageSource src("scene", "@animate=off@format=rgbd");
  Image rgbd = src.grab();
  ICL_TEST_EQ(rgbd.getChannels(), 4);
  ICL_TEST_EQ((int)rgbd.getDepth(), (int)depth32f);

  const Img32f &x = rgbd.as<icl32f>();
  bool colorInRange = true, depthHit = false;
  for (int i = 0; i < x.getDim(); ++i) {
    const float r = x.begin(0)[i];
    if (r < -0.5f || r > 255.5f) colorInRange = false;
    if (x.begin(3)[i] > 1.f) depthHit = true;
  }
  ICL_TEST_EQ(colorInRange, true);
  ICL_TEST_EQ(depthHit, true);
}

// End-to-end: the depth image + the camera from metadata unproject into a point
// cloud (the depth -> RGBD -> cloud loop the source exists to enable).
ICL_REGISTER_TEST("io.scenesource.reconstruct_cloud", "depth + metadata camera -> point cloud")
{
  ImageSource src("scene", "@animate=off");
  Image depth = src.grab();
  std::istringstream is(depth.ptr()->getMetaData());
  Camera cam; is >> cam;

  geom::PointCloudCreator pcc(cam, geom::PointCloudCreator::DistanceToCamPlane);
  geom::PointCloudObject cloud(640, 480, true);
  pcc.create(depth.as<icl32f>(), cloud);
  ICL_TEST_EQ(cloud.getDim(), 640 * 480);

  // Count reconstructed points lying within the scene (background pixels map to
  // the camera centre, ~680mm away, so they fall outside this radius).
  auto xyz = cloud.selectXYZ();
  int realHits = 0;
  for (int i = 0; i < cloud.getDim(); ++i) {
    auto &p = xyz[i];
    if (!std::isfinite(p[0])) continue;
    const float dist = std::sqrt(p[0]*p[0] + p[1]*p[1] + p[2]*p[2]);
    if (dist < 550.f) ++realHits;
  }
  ICL_TEST_EQ(realHits > 1000, true);
}

// The shared helper used by icl-point-cloud-{viewer,pipe}: PointCloudSource
// wraps the ImageSource, resolves the camera from metadata, and reconstructs.
ICL_REGISTER_TEST("io.scenesource.pointcloudsource", "PointCloudSource grabs scene -> coloured cloud")
{
  geom2::PointCloudSource src;
  src.init("scene", "@animate=off@format=rgbd");
  geom2::PointCloud cloud;
  const bool ok = src.grab(cloud);
  ICL_TEST_EQ(ok, true);
  ICL_TEST_EQ(src.hasCamera(), true);
  ICL_TEST_EQ(cloud.getDim(), 640 * 480);
  ICL_TEST_EQ(cloud.supports(geom2::PointCloud::RGBA32f), true);   // rgbd -> colour

  auto xyz = cloud.selectXYZ();
  int realHits = 0;
  for (int i = 0; i < cloud.getDim(); ++i) {
    auto &p = xyz[i];
    const float d = std::sqrt(p[0]*p[0] + p[1]*p[1] + p[2]*p[2]);
    if (std::isfinite(d) && d > 1.f && d < 550.f) ++realHits;
  }
  ICL_TEST_EQ(realHits > 1000, true);
}

// Geometric keep/remove filters on the reconstructed cloud.
ICL_REGISTER_TEST("io.scenesource.geom2_filters", "PointCloud::filterSphere/filterBox keep/remove points")
{
  geom2::PointCloudSource src;
  src.init("scene", "@animate=off");

  auto countValid = [](geom2::PointCloud &c) {
    auto xyz = c.selectXYZ();
    int n = 0;
    for (int i = 0; i < c.getDim(); ++i) {
      auto &p = xyz[i];
      if (p[0] != 0 || p[1] != 0 || p[2] != 0) ++n;
    }
    return n;
  };

  // keep only a sphere around the content -> fewer (but some) valid points
  geom2::PointCloud a;
  src.grab(a);
  const int before = countValid(a);
  a.filterSphere(geom2::Vec(0, 0, 50, 1), 200.f, /*keepInside=*/true);
  const int afterSphere = countValid(a);
  ICL_TEST_EQ(before > 0, true);
  ICL_TEST_EQ(afterSphere > 0 && afterSphere < before, true);

  // remove the inside of a box -> also fewer valid points
  geom2::PointCloud b;
  src.grab(b);
  b.filterBox(geom2::Vec(0, 0, 50, 1), geom2::Vec(120, 120, 120, 0), /*keepInside=*/false);
  const int afterBox = countValid(b);
  ICL_TEST_EQ(afterBox > 0 && afterBox < before, true);
}

// Camera-based near/far filter: keeps only points whose depth from the camera
// is within range (the cloud has no depth, so this needs the camera).
ICL_REGISTER_TEST("io.scenesource.geom2_depthrange", "PointCloud::filterDepthRange keeps in-range points")
{
  geom2::PointCloudSource src;
  src.init("scene", "@animate=off");
  geom2::PointCloud c;
  src.grab(c);
  const Camera cam = src.getCamera();

  auto xyz = c.selectXYZ();
  int before = 0;
  for (int i = 0; i < c.getDim(); ++i) {
    auto &p = xyz[i];
    if (p[0] != 0 || p[1] != 0 || p[2] != 0) ++before;
  }

  const float maxD = 650.f;
  c.filterDepthRange(cam, 0.f, maxD);

  // camera forward for the verification
  auto o = cam.getPosition();
  auto f = cam.getNorm();
  const float fn = std::sqrt(f[0]*f[0] + f[1]*f[1] + f[2]*f[2]);

  int after = 0; bool allInRange = true;
  for (int i = 0; i < c.getDim(); ++i) {
    auto &p = xyz[i];
    if (p[0] == 0 && p[1] == 0 && p[2] == 0) continue;
    ++after;
    const float depth = ((p[0]-o[0])*f[0] + (p[1]-o[1])*f[1] + (p[2]-o[2])*f[2]) / fn;
    if (depth > maxD + 1.f) allInRange = false;
  }
  ICL_TEST_EQ(after > 0 && after < before, true);
  ICL_TEST_EQ(allInRange, true);
}

// The geom2-native consumer path: PointCloud::unprojectDepth (what
// icl-point-cloud-viewer uses) reconstructs a cloud from depth + camera.
ICL_REGISTER_TEST("io.scenesource.geom2_unproject", "geom2 PointCloud::unprojectDepth reconstructs the scene")
{
  ImageSource src("scene", "@animate=off");
  Image depth = src.grab();
  std::istringstream is(depth.ptr()->getMetaData());
  Camera cam; is >> cam;

  geom2::PointCloud cloud;
  cloud.unprojectDepth(depth.as<icl32f>(), cam, /*distToCamPlane=*/true);
  ICL_TEST_EQ(cloud.getDim(), 640 * 480);
  ICL_TEST_EQ(cloud.supports(geom2::PointCloud::XYZ), true);

  auto xyz = cloud.selectXYZ();
  int realHits = 0;
  for (int i = 0; i < cloud.getDim(); ++i) {
    auto &p = xyz[i];
    const float d = std::sqrt(p[0]*p[0] + p[1]*p[1] + p[2]*p[2]);
    if (std::isfinite(d) && d > 1.f && d < 550.f) ++realHits;
  }
  ICL_TEST_EQ(realHits > 1000, true);
}
