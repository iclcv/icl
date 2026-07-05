// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// CoplanarPointPoseEstimator::getPoses — the two-solution (IPPE) planar pose.
// A single planar marker's pose has a two-fold "flip" ambiguity (tilted toward
// vs away) that reprojects almost identically under weak perspective. getPoses
// returns both hypotheses best-first; the error ratio flags ambiguity. Verified
// on synthetic projections of a tilted square: the best solution recovers the
// truth, and a weak-perspective view genuinely surfaces the second solution.

#include "harness/Test.h"
#include <icl/cv3d/Camera.h>
#include <icl/cv3d/CoplanarPointPoseEstimator.h>
#include <cmath>

using namespace icl;
using namespace icl::geom;
using icl::utils::Point32f;
using icl::utils::Size;
using icl::math::FixedMatrix;

namespace {
  // marker->world pose: tilt about world-x by angle a (rad), translation tw.
  // FixedMatrix is (col,row): columns are the x/y/z axes, col 3 the translation.
  FixedMatrix<float,4,4> markerPose(float a, const Vec &tw) {
    FixedMatrix<float,4,4> T = FixedMatrix<float,4,4>::id();
    const float c = std::cos(a), s = std::sin(a);
    T(1,1)= c; T(1,2)= s;        // y-axis = (0, c, s)
    T(2,1)=-s; T(2,2)= c;        // z-axis = (0,-s, c)
    T(3,0)=tw[0]; T(3,1)=tw[1]; T(3,2)=tw[2];
    return T;
  }
  // project the 4 corners of a side-100 square marker at pose T through cam
  void projectMarker(const Camera &cam, const FixedMatrix<float,4,4> &T,
                     Point32f model[4], Point32f img[4]) {
    const Point32f m[4] = {{-50,-50},{50,-50},{50,50},{-50,50}};
    for (int i = 0; i < 4; ++i) {
      model[i] = m[i];
      const Vec Xw = T * Vec(m[i].x, m[i].y, 0, 1);
      img[i] = cam.project(Xw);
    }
  }
  // |cos| between two poses' marker normals (z-axis = col 2)
  float normalDot(const FixedMatrix<float,4,4> &A, const FixedMatrix<float,4,4> &B) {
    return A(2,0)*B(2,0) + A(2,1)*B(2,1) + A(2,2)*B(2,2);
  }
}

// The best of the two solutions recovers the true pose (synthetic exact data →
// only the true pose reprojects with ~0 error; the flip is worse under
// perspective), with the correct marker normal.
ICL_REGISTER_TEST("geom.coplanarpose.getposes_recovers_truth",
                  "getPoses' best solution recovers the true planar pose")
{
  Camera cam = Camera::lookAt(Vec(0,0,800,1), Vec(0,0,0,1), Vec(0,1,0,1), Size::VGA, 30.f);
  const float a = 0.55f;                                   // ~31 deg tilt
  const FixedMatrix<float,4,4> T = markerPose(a, Vec(0,0,0,1));
  Point32f model[4], img[4];
  projectMarker(cam, T, model, img);

  CoplanarPointPoseEstimator est(CoplanarPointPoseEstimator::worldFrame);
  const auto ps = est.getPoses(4, model, img, cam);
  std::cout << "[coplanarpose] recover: n=" << ps.size() << " err0=" << ps[0].error
            << (ps.size()>1 ? std::string(" err1=")+std::to_string(ps[1].error) : "") << std::endl;

  ICL_TEST_TRUE(ps.size() >= 1);
  ICL_TEST_TRUE(ps[0].error < 0.6f);                       // best ≈ exact
  ICL_TEST_TRUE(std::fabs(normalDot(ps[0].pose, T)) > 0.99f);   // correct tilt (sign-agnostic)
}

// Weak perspective (far camera + narrow FOV + modest tilt): the flip is a
// near-equal explanation → getPoses surfaces TWO distinct, low-error solutions.
ICL_REGISTER_TEST("geom.coplanarpose.getposes_surfaces_flip",
                  "getPoses surfaces the second (flip) solution under weak perspective")
{
  Camera cam = Camera::lookAt(Vec(0,0,4000,1), Vec(0,0,0,1), Vec(0,1,0,1), Size::VGA, 6.f);
  const float a = 15.f*(float)M_PI/180.f;
  const FixedMatrix<float,4,4> T = markerPose(a, Vec(0,0,0,1));
  Point32f model[4], img[4];
  projectMarker(cam, T, model, img);

  CoplanarPointPoseEstimator est(CoplanarPointPoseEstimator::worldFrame);
  const auto ps = est.getPoses(4, model, img, cam);
  std::cout << "[coplanarpose] flip: n=" << ps.size() << " err0=" << ps[0].error
            << (ps.size()>1 ? std::string(" err1=")+std::to_string(ps[1].error) : "")
            << " ratio=" << (ps.size()>1 ? ps[0].error/std::max(1e-3f,ps[1].error) : 0.f) << std::endl;

  ICL_TEST_EQ((int)ps.size(), 2);                          // ambiguous → two candidates
  ICL_TEST_TRUE(ps[0].error < 1.0f);                       // best ≈ exact
  ICL_TEST_TRUE(ps[1].error < 1.5f);                       // flip reprojects nearly as well
  ICL_TEST_TRUE(normalDot(ps[0].pose, ps[1].pose) < 0.99f);// genuinely different tilt
}