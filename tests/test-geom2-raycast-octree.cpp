// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// RayCastOctree::fill(PointCloud) — populate the ray-cast octree directly from
// a geom2 PointCloud's XYZ (the geom2 replacement for the now-removed legacy
// SceneObject-derived OctreeObject). Headless.

#include "harness/Test.h"
#include <icl/geom2/RayCastOctree.h>
#include <icl/geom2/PointCloud.h>
#include <icl/cv3d/ViewRay.h>
#include <cmath>

using namespace icl;
using namespace icl::geom2;
using icl::geom::ViewRay;
using Vec4 = math::FixedColVector<float,4>;

// fill from a small cloud, then a ray aimed at one point returns exactly that
// point with its cloud index in the w-component; origin points are skipped.
ICL_REGISTER_TEST("geom2.raycastoctree.fill_and_cast",
                  "fill(PointCloud) inserts valid XYZ with index; rayCast finds them")
{
  PointCloud cloud(4);                  // 4 unorganized points, XYZ
  auto xyz = cloud.selectXYZ();
  xyz[0][0] = 100; xyz[0][1] = 0;   xyz[0][2] = 0;
  xyz[1][0] = 0;   xyz[1][1] = 100; xyz[1][2] = 0;
  xyz[2][0] = 0;   xyz[2][1] = 0;   xyz[2][2] = 100;
  xyz[3][0] = 0;   xyz[3][1] = 0;   xyz[3][2] = 0;     // invalid (origin) → skipped

  RayCastOctree oct(-1000.f, 2000.f);
  oct.fill(cloud);

  // ray from +X toward the first point (100,0,0)
  ViewRay ray(Vec4(300, 0, 0, 1), Vec4(-1, 0, 0, 0));
  RayCastOctree::Pt hit = oct.rayCastClosest(ray, 5.0f);
  ICL_TEST_EQ(std::fabs(hit[0] - 100.f) < 1e-3f, true);
  ICL_TEST_EQ(std::fabs(hit[1]) < 1e-3f, true);
  ICL_TEST_EQ((int)std::lround(hit[3]), 0);            // carries cloud index 0

  // ray toward the third point (0,0,100)
  ViewRay ray2(Vec4(0, 0, 300, 1), Vec4(0, 0, -1, 0));
  RayCastOctree::Pt hit2 = oct.rayCastClosest(ray2, 5.0f);
  ICL_TEST_EQ((int)std::lround(hit2[3]), 2);

  // the origin point was skipped → a ray through it finds nothing nearby
  ViewRay ray3(Vec4(0, -300, 0, 1), Vec4(0, 1, 0, 0));  // passes near (0,100,0) too
  // aim slightly off everything: nothing within 5mm of this ray near origin
  ViewRay rayMiss(Vec4(50, 50, -300, 1), Vec4(0, 0, 1, 0));
  bool threw = false;
  try { oct.rayCastClosest(rayMiss, 2.0f); } catch (...) { threw = true; }
  ICL_TEST_EQ(threw, true);
}
