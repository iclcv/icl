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
