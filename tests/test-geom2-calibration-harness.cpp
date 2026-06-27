// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Closed-loop calibration-accuracy harness (Phase A, projection-based — no
// renderer/detector yet). A known calibration object is projected through a
// known ("ground-truth") camera to synthesise perfect 2D↔3D correspondences;
// Gaussian pixel noise is added; the camera is then recovered both ways and the
// error measured against truth. This reproduces — and quantifies — the
// historical "depth drift" of the single-object joint DLT and proves the
// intrinsics/extrinsics decoupling fix that ICL's own Camera docs recommend.

#include "harness/Test.h"
#include <icl/geom/Camera.h>
#include <vector>
#include <cmath>

using namespace icl;
using icl::geom::Camera;
using icl::geom::Vec;
using icl::utils::Point32f;
using icl::utils::Size;

namespace {
  // deterministic PRNG (reproducible test, no dependency on global RNG state)
  struct Rng {
    uint64_t s;
    explicit Rng(uint64_t seed) : s(seed ? seed : 1) {}
    uint64_t next() { s ^= s << 13; s ^= s >> 7; s ^= s << 17; return s; }
    double uniform() { return (next() >> 11) * (1.0 / 9007199254740992.0); }
    double gauss() {  // Box–Muller
      double u1 = std::max(1e-12, uniform()), u2 = uniform();
      return std::sqrt(-2*std::log(u1)) * std::cos(2*M_PI*u2);
    }
  };

  // a genuinely 3D calibration object: 3×3 grids on two z-planes (non-coplanar)
  std::vector<Vec> build3DTarget() {
    std::vector<Vec> P;
    for (float z : {0.f, 120.f})
      for (int gy = -1; gy <= 1; ++gy)
        for (int gx = -1; gx <= 1; ++gx)
          P.push_back(Vec(gx*80.f, gy*80.f, z, 1));
    return P;
  }

  std::vector<Point32f> projectNoisy(const Camera &cam, const std::vector<Vec> &Xws,
                                     double sigmaPx, Rng &rng) {
    std::vector<Point32f> xis;
    for (const Vec &X : Xws) {
      Point32f p = cam.project(X);
      xis.push_back(Point32f(p.x + (float)(rng.gauss()*sigmaPx),
                             p.y + (float)(rng.gauss()*sigmaPx)));
    }
    return xis;
  }

  float posDist(const Vec &a, const Vec &b) {
    return std::sqrt((a[0]-b[0])*(a[0]-b[0]) + (a[1]-b[1])*(a[1]-b[1]) + (a[2]-b[2])*(a[2]-b[2]));
  }

  struct Stats { double mean = 0, std = 0; };
  Stats meanStd(const std::vector<double> &v) {
    Stats s; for (double x : v) s.mean += x; s.mean /= v.size();
    for (double x : v) s.std += (x - s.mean)*(x - s.mean);
    s.std = std::sqrt(s.std / v.size());
    return s;
  }
}

// Sanity: with perfect (noise-free) correspondences the joint DLT recovers the
// ground-truth camera position essentially exactly — the harness + math work.
ICL_REGISTER_TEST("geom2.calibharness.perfect_recovery",
                  "joint DLT recovers a known camera from perfect correspondences")
{
  const std::vector<Vec> Xws = build3DTarget();
  Camera truth = Camera::lookAt(Vec(150, -200, 700, 1), Vec(0,0,40,1), Vec(0,0,1,1),
                                Size::VGA, 35.0f);
  Rng rng(12345);
  std::vector<Point32f> xis = projectNoisy(truth, Xws, 0.0, rng);   // no noise
  Camera est = Camera::calibrate_pinv(Xws, xis, 1, true);
  ICL_TEST_EQ(posDist(est.getPosition(), truth.getPosition()) < 2.0f, true);   // <2mm
}

// reduce a camera to its distance-from-object (the quantity that "drifts")
static double camDepth(const Camera &c) {
  const Vec p = c.getPosition();
  return std::sqrt(p[0]*p[0] + p[1]*p[1] + p[2]*p[2]);
}

// Core experiment #1 — the depth drift is a CONDITIONING effect: the same
// single-object joint DLT (calibrate_pinv) under the same sub-pixel noise gives
// a small, tight depth estimate when the object is near (strong perspective) but
// a large, wildly-scattered one when it is far (near-isometric → focal↔distance
// ambiguity). This reproduces & quantifies ICL's documented z-drift using only
// calibrate_pinv, and is exactly why intrinsics must come from a well-filled,
// near view — the motivation for the planar multi-view path.
ICL_REGISTER_TEST("geom2.calibharness.depth_drift_is_conditioning",
                  "joint-DLT depth error explodes for a far (isometric) object vs a near one")
{
  const std::vector<Vec> Xws = build3DTarget();
  const double sigma = 0.4;   // sub-pixel corner noise
  auto trial_depths = [&](float camZ, uint64_t seed) {
    Camera truth = Camera::lookAt(Vec(0,0,camZ,1), Vec(0,0,0,1), Vec(0,1,0,1), Size::VGA, 35.0f);
    const double truthDepth = camDepth(truth);
    Rng rng(seed);
    std::vector<double> err;
    for (int t = 0; t < 25; ++t) {
      std::vector<Point32f> xis = projectNoisy(truth, Xws, sigma, rng);
      err.push_back(std::fabs(camDepth(Camera::calibrate_pinv(Xws, xis, 1, true)) - truthDepth));
    }
    return meanStd(err);
  };
  Stats near = trial_depths(700.f,  111);   // object fills the frame
  Stats far  = trial_depths(3000.f, 222);   // object small & near-isometric
  std::cout << "[calib-harness] near(700mm)  depth-err mean=" << near.mean << " std=" << near.std << "\n"
            << "[calib-harness] far(3000mm)  depth-err mean=" << far.mean  << " std=" << far.std
            << std::endl;
  // the far/isometric case drifts dramatically more (both bias and spread)
  ICL_TEST_EQ(far.std  > near.std  * 3.0, true);
  ICL_TEST_EQ(far.mean > near.mean * 3.0, true);
}

// Core experiment #2 — the decoupling FIX, validated. The investigation of the
// calibrate_extrinsic "divergence" found the root cause: its internal LINEAR SVD
// seed is non-robust (cheirality/scale → a camera at the wrong sign & 3× scale),
// and the LMA cannot escape that bad basin. But the LMA itself
// (optimize_camera_calibration_lma) IS the correct fixed-intrinsics /
// extrinsic-only solver (it varies only the 6 extrinsic params, intrinsics held
// in P), and it has a WIDE convergence basin: empirically a seed pose off by
// ±600mm at 3m still converges to ~15mm. So the fix is simply to seed it from a
// reliable pose (a homography/PnP init — Phase B), NOT the broken linear solve.
//
// Here we model a homography-quality seed by perturbing the truth pose by ±200mm
// and assert the decoupled result crushes the joint DLT's depth drift.
ICL_REGISTER_TEST("geom2.calibharness.decoupled_seeded_beats_drift",
                  "fixed-intrinsics extrinsic LMA from a rough seed beats the joint-DLT depth drift")
{
  const std::vector<Vec> Xws = build3DTarget();
  Camera truth = Camera::lookAt(Vec(0,0,3000,1), Vec(0,0,0,1), Vec(0,1,0,1), Size::VGA, 25.0f);
  const double truthDepth = camDepth(truth);
  const double sigma = 0.4, seedPert = 200.0;
  Rng rng(2468);
  std::vector<double> jointErr, decErr;
  for (int t = 0; t < 15; ++t) {
    std::vector<Point32f> xis = projectNoisy(truth, Xws, sigma, rng);
    jointErr.push_back(std::fabs(camDepth(Camera::calibrate_pinv(Xws, xis, 1, true)) - truthDepth));
    Camera seed = truth;   // fixed (correct) intrinsics; pose seeded roughly
    const Vec tp = truth.getPosition();
    seed.setPosition(Vec(tp[0] + (float)(rng.gauss()*seedPert),
                         tp[1] + (float)(rng.gauss()*seedPert),
                         tp[2] + (float)(rng.gauss()*seedPert), 1));
    Camera dec = Camera::optimize_camera_calibration_lma(Xws, xis, seed);
    decErr.push_back(std::fabs(camDepth(dec) - truthDepth));
  }
  Stats j = meanStd(jointErr), d = meanStd(decErr);
  std::cout << "[calib-harness] joint DLT       depth-err mean=" << j.mean << " std=" << j.std << "\n"
            << "[calib-harness] decoupled+seed  depth-err mean=" << d.mean << " std=" << d.std
            << std::endl;
  ICL_TEST_EQ(d.mean < j.mean * 0.5, true);   // decoupled at least 2x better
}
