// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// PlaneFitter — the ModelFitter<Vec,PlaneModel> face (least-squares plane fit),
// the generic unstructured-point counterpart to RansacPlaneFitter's tuned OpenCL
// pipeline. Guards (a) exact plane recovery from clean points, and (b) that
// wrapped in the generic math::RobustFitter it rejects gross outliers.

#include "harness/Test.h"
#include <icl/cv3d/pose/RansacPlaneFitter.h>
#include <icl/math/fit/RobustFitter.h>
#include <cmath>
#include <vector>

using namespace icl;
using namespace icl::cv3d;
using icl::math::RobustFitter;

namespace {
  Vec normed(const Vec &v) {
    const float l = std::sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
    return Vec(v[0]/l, v[1]/l, v[2]/l, 0);
  }
  float dot3(const Vec &a, const Vec &b) { return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]; }

  // A grid of points lying exactly on the plane {p : p·n == d}.
  std::vector<Vec> planeCloud(const Vec &n, float d) {
    // an in-plane orthonormal basis (u,v)
    Vec a = (std::fabs(n[0]) < 0.9f) ? Vec(1,0,0,0) : Vec(0,1,0,0);
    Vec u = normed(Vec(a[1]*n[2]-a[2]*n[1], a[2]*n[0]-a[0]*n[2], a[0]*n[1]-a[1]*n[0], 0));
    Vec v = normed(Vec(n[1]*u[2]-n[2]*u[1], n[2]*u[0]-n[0]*u[2], n[0]*u[1]-n[1]*u[0], 0));
    const Vec base(n[0]*d, n[1]*d, n[2]*d, 1);   // base·n == d
    std::vector<Vec> pts;
    for (int i = -3; i <= 3; ++i)
      for (int j = -3; j <= 3; ++j) {
        const float s = 30.f*i, t = 25.f*j;
        pts.push_back(Vec(base[0]+s*u[0]+t*v[0], base[1]+s*u[1]+t*v[1], base[2]+s*u[2]+t*v[2], 1));
      }
    return pts;
  }
}

// Clean points: PlaneFitter recovers the plane (normal up to sign, zero residual).
ICL_REGISTER_TEST("cv3d.planefitter.recovers_known_plane",
                  "PlaneFitter recovers a known plane from clean points")
{
  const Vec n = normed(Vec(1,2,3,0));
  const float d = 40.f;
  const std::vector<Vec> pts = planeCloud(n, d);

  PlaneFitter fit;
  ICL_TEST_EQ(fit.minSamples(), 3);
  const PlaneModel m = fit.fit(pts);

  // normal recovered up to sign, and every point lies on the fitted plane
  ICL_TEST_TRUE(std::fabs(std::fabs(dot3(m.n0, n)) - 1.f) < 1e-3f);
  double maxRes = 0;
  for (const Vec &p : pts) maxRes = std::max(maxRes, fit.residual(m, p));
  ICL_TEST_TRUE(maxRes < 1e-2);
}

// RobustFitter<Vec,PlaneModel> rejects off-plane outliers.
ICL_REGISTER_TEST("cv3d.planefitter.robustfitter_rejects_outliers",
                  "RobustFitter<Vec,PlaneModel> over PlaneFitter rejects outliers")
{
  const Vec n = normed(Vec(-2,1,4,0));
  const float d = -15.f;
  std::vector<Vec> pts = planeCloud(n, d);   // 49 clean points
  // shove five points far off the plane (+200mm along the normal)
  for (int i : {3, 11, 20, 33, 44})
    pts[i] = Vec(pts[i][0]+200*n[0], pts[i][1]+200*n[1], pts[i][2]+200*n[2], 1);

  PlaneFitter base;
  RobustFitter<Vec, PlaneModel> robust(&base, /*inlierThresh*/ 2.0, 0.99,
                                       /*iterations*/ 300, "msac", /*localOpt*/ true);
  const PlaneModel m = robust.fit(pts);

  // recovered plane matches the clean geometry, outliers excluded
  ICL_TEST_TRUE(std::fabs(std::fabs(dot3(m.n0, n)) - 1.f) < 1e-2f);
  ICL_TEST_TRUE((int)robust.inliers().size() >= 40);   // ~44 clean survive
  for (const Vec &c : robust.inliers()) ICL_TEST_TRUE(base.residual(m, c) < 2.0);
}
