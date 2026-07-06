// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// RigidTransformFitter — the ModelFitter<PointPair,Mat> face over
// RigidTransformEstimator::map. Guards that (a) it recovers a known rigid
// transform from clean 3D-3D correspondences, and (b) wrapped in the generic
// math::RobustFitter it rejects gross outliers and still recovers the transform.

#include "harness/Test.h"
#include <icl/cv3d/pose/RigidTransformEstimator.h>
#include <icl/math/fit/RobustFitter.h>
#include <icl/math/la/FixedMatrix.h>
#include <cmath>
#include <vector>

using namespace icl;
using namespace icl::cv3d;
using icl::math::FixedMatrix;
using icl::math::RobustFitter;

namespace {
  // rigid transform: rotate about world-x by a (rad) + translation t (col 3).
  Mat rigid(float a, const Vec &t) {
    Mat T = Mat::id();
    const float c = std::cos(a), s = std::sin(a);
    T(1,1)= c; T(1,2)= s; T(2,1)=-s; T(2,2)= c;
    T(0,3)=t[0]; T(1,3)=t[1]; T(2,3)=t[2];
    return T;
  }
  // a spread-out, non-collinear source cloud
  std::vector<Vec> sourceCloud() {
    return { Vec(0,0,0,1), Vec(100,0,0,1), Vec(0,100,0,1), Vec(0,0,100,1),
             Vec(50,50,20,1), Vec(-30,40,-60,1), Vec(80,-70,10,1) };
  }
  float frob(const Mat &A, const Mat &B) {
    float e = 0; for (int i=0;i<16;++i) e += (A[i]-B[i])*(A[i]-B[i]);
    return std::sqrt(e);
  }
}

// Clean correspondences: RigidTransformFitter::fit recovers the exact transform.
ICL_REGISTER_TEST("cv3d.rigidtransform.recovers_known_transform",
                  "RigidTransformFitter recovers a known rigid transform from clean matches")
{
  const Mat T = rigid(0.5f, Vec(35,-20,80,1));
  const std::vector<Vec> src = sourceCloud();

  std::vector<PointPair> corr;
  for (const Vec &s : src) corr.push_back(PointPair(s, T * s));

  RigidTransformFitter fit;                       // RigidBody
  ICL_TEST_EQ(fit.minSamples(), 3);
  const Mat Trec = fit.fit(corr);

  // every source point maps onto its target
  double maxRes = 0;
  for (const PointPair &p : corr) maxRes = std::max(maxRes, fit.residual(Trec, p));
  ICL_TEST_TRUE(maxRes < 1e-3);
  ICL_TEST_TRUE(frob(Trec, T) < 1e-3f);
}

// Wrapped in the generic RobustFitter, it rejects gross outliers.
ICL_REGISTER_TEST("cv3d.rigidtransform.robustfitter_rejects_outliers",
                  "RobustFitter<PointPair,Mat> over RigidTransformFitter rejects outliers")
{
  const Mat T = rigid(-0.35f, Vec(-40,15,60,1));
  const std::vector<Vec> src = sourceCloud();

  std::vector<PointPair> corr;
  for (const Vec &s : src) corr.push_back(PointPair(s, T * s));
  // corrupt three targets with gross errors
  corr[1].to += Vec(300,-250,400,0);
  corr[4].to += Vec(-500,200,-150,0);
  corr[6].to += Vec(220,220,-330,0);

  RigidTransformFitter base;                       // RigidBody, minSamples 3
  RobustFitter<PointPair, Mat> robust(&base, /*inlierThresh*/ 5.0, 0.99,
                                      /*iterations*/ 200, "msac", /*localOpt*/ true);
  const Mat Trec = robust.fit(corr);

  // the four clean correspondences must be recovered tightly...
  for (int i : {0,2,3,5}) ICL_TEST_TRUE(base.residual(Trec, corr[i]) < 1e-2);
  // ...and the three corrupted ones flagged as outliers
  ICL_TEST_TRUE((int)robust.inliers().size() >= 4);
  for (int i : {0,2,3,5}) ICL_TEST_TRUE(base.residual(Trec, corr[i]) < 5.0);
  ICL_TEST_TRUE(frob(Trec, T) < 1.0f);
}
