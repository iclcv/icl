// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// RobustPoseEstimator — RANSAC over 2D-3D coplanar correspondences that rejects
// gross outliers and recovers the planar target's pose (base algorithm:
// HomographyBasedOnly, wrapped in math::RobustFitter). Guards the RANSAC-core
// migration (old RansacFitter -> RobustFitter) end to end: a known pose is
// projected onto a grid, a few image points are corrupted, and the recovered
// pose must reproject the CLEAN inliers tightly.

#include "harness/Test.h"
#include <icl/cv3d/Camera.h>
#include <icl/cv3d/pose/RobustPoseEstimator.h>
#include <cmath>
#include <vector>

using namespace icl;
using namespace icl::cv3d;
using icl::utils::Point32f;
using icl::utils::Size;
using icl::math::FixedMatrix;

namespace {
  // planar target pose: tilt about world-x by a (rad) + translation tw.
  // NB: translation goes in COLUMN 3 (T*v convention) — putting it in row 3 is a
  // silent trap that only bites once tw != 0 (see git history of this test).
  FixedMatrix<float,4,4> targetPose(float a, const Vec &tw) {
    FixedMatrix<float,4,4> T = FixedMatrix<float,4,4>::id();
    const float c = std::cos(a), s = std::sin(a);
    T(1,1)= c; T(1,2)= s; T(2,1)=-s; T(2,2)= c;
    T(0,3)=tw[0]; T(1,3)=tw[1]; T(2,3)=tw[2];
    return T;
  }
}

ICL_REGISTER_TEST("cv3d.robustpose.rejects_outliers_recovers_pose",
                  "RobustPoseEstimator recovers the pose and rejects gross outliers")
{
  Camera cam = Camera::lookAt(Vec(0,0,800,1), Vec(0,0,0,1), Vec(0,1,0,1), Size::VGA, 30.f);
  const FixedMatrix<float,4,4> T = targetPose(0.4f, Vec(20,-10,0,1));

  // 5x5 grid of coplanar model points over a 200x200 target, projected at pose T
  std::vector<Point32f> model, image, clean;
  for (int gy = 0; gy < 5; ++gy) {
    for (int gx = 0; gx < 5; ++gx) {
      const float mx = -100 + 50*gx, my = -100 + 50*gy;
      model.push_back(Point32f(mx, my));
      const Point32f q = cam.project(T * Vec(mx, my, 0, 1));
      image.push_back(q);
      clean.push_back(q);
    }
  }

  // corrupt 5 of the 25 image points with gross offsets (outliers)
  const int outlierIdx[5] = {2, 7, 11, 18, 23};
  for (int k = 0; k < 5; ++k) image[outlierIdx[k]] += Point32f(60, -45);

  RobustPoseEstimator pe(cam);              // defaults: 200 iters, 4 pts, maxErr 5
  pe.setStoreLastConsensusSet(true);
  RobustPoseEstimator::Result r = pe.fit(model, image);

  ICL_TEST_TRUE(r.found);

  // the recovered pose must reproject the 20 CLEAN inliers tightly (outliers did
  // not corrupt the fit)
  double meanErr = 0; int nIn = 0;
  for (size_t i = 0; i < model.size(); ++i) {
    bool isOutlier = false;
    for (int k = 0; k < 5; ++k) if ((int)i == outlierIdx[k]) isOutlier = true;
    if (isOutlier) continue;
    const Point32f q = cam.project(r.T * Vec(model[i].x, model[i].y, 0, 1));
    meanErr += q.distanceTo(clean[i]); ++nIn;
  }
  meanErr /= nIn;
  std::cout << "[robustpose] mean inlier reproj err=" << meanErr << "px consensus="
            << pe.getLastConsensusSet().size() << std::endl;
  ICL_TEST_TRUE(meanErr < 1.0);                              // pose recovered despite outliers
  ICL_TEST_TRUE(pe.getLastConsensusSet().size() <= 21);      // 5 gross outliers rejected
}
