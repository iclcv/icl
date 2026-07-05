// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// ICP registration (cv3d) — the unified class built on the octree C++ backend.
// Synthetic, headless: a random point cloud is displaced by a known rigid
// transform, and ICP must recover it (and thus align source onto target).
// Folds in the asserts the old apps/icp3d-test.cpp only eyeballed via SHOW().

#include "harness/Test.h"
#include <icl/cv3d/icp/ICP.h>
#include <icl/math/transform/HomogeneousMath.h>
#include <icl/utils/Random.h>

using namespace icl;
using namespace icl::cv3d;
using icl::cv3d::Vec;
using icl::math::Mat4;

namespace {
  // a reproducible random cloud in [-50,50]^3 (homogeneous, w=1)
  std::vector<Vec> randomCloud(int n, unsigned int seed) {
    utils::randomSeed(seed);
    std::vector<Vec> pts;
    pts.reserve(n);
    for (int i = 0; i < n; ++i) {
      pts.push_back(Vec(utils::random(-50.0, 50.0),
                        utils::random(-50.0, 50.0),
                        utils::random(-50.0, 50.0), 1));
    }
    return pts;
  }

  // mean nearest-neighbour distance from src to a reference cloud (alignment
  // quality; here src and ref are index-aligned so it's a direct residual)
  double meanResidual(const std::vector<Vec> &a, const std::vector<Vec> &b) {
    double s = 0;
    for (size_t i = 0; i < a.size(); ++i) s += icl::math::dist3(a[i], b[i]);
    return s / a.size();
  }
}

ICL_REGISTER_TEST("cv3d.icp.recovers_known_transform",
                  "ICP recovers a known rigid transform and aligns the clouds")
{
  const std::vector<Vec> target = randomCloud(200, 42);

  // displace the target by a known rigid transform to make the source
  const Mat4 T = icl::math::create_hom_4x4<float>(0.15f, -0.1f, 0.08f, 5, -5, 2);
  std::vector<Vec> source;
  source.reserve(target.size());
  for (const Vec &v : target) source.push_back(T * v);

  ICP icp(50, 20.0f, 1e-4);
  std::vector<Vec> aligned;
  ICP::Result r = icp.apply(target, source, aligned);

  std::cout << "[icp] iterations=" << r.iterations << " error=" << r.error
            << std::endl;

  // ICP transforms source→target, so it should invert T: aligned ≈ target
  ICL_TEST_LT(r.error, 0.5);
  ICL_TEST_LT(meanResidual(aligned, target), 0.5);

  // the recovered transform composed with T should be (near) identity
  const Mat4 residualT = r.transformation * T;
  const Mat4 I = Mat4::id();
  double maxAbs = 0;
  for (int i = 0; i < 16; ++i) maxAbs = std::max(maxAbs, (double)std::abs(residualT[i] - I[i]));
  ICL_TEST_LT(maxAbs, 0.02);
}

ICL_REGISTER_TEST("cv3d.icp.already_aligned_is_noop",
                  "identical clouds converge immediately with ~zero error")
{
  const std::vector<Vec> target = randomCloud(100, 7);
  ICP icp;
  std::vector<Vec> aligned;
  ICP::Result r = icp.apply(target, target, aligned);
  ICL_TEST_NEAR(r.error, 0.0, 1e-4);
  ICL_TEST_LT(meanResidual(aligned, target), 1e-3);
}

ICL_REGISTER_TEST("cv3d.icp.empty_inputs_are_safe",
                  "empty source/target return the default (identity) result")
{
  ICP icp;
  std::vector<Vec> out;
  ICP::Result r = icp.apply({}, {}, out);
  ICL_TEST_EQ((int)r.iterations, 0);
  ICL_TEST_TRUE(out.empty());
}

ICL_REGISTER_TEST("cv3d.icp.default_backend_is_octree",
                  "a fresh ICP owns an OctreeNN backend, and it is swappable")
{
  ICP icp;
  ICL_TEST_TRUE(dynamic_cast<OctreeNN*>(icp.getBackend()) != nullptr);

  auto custom = std::make_shared<OctreeNN>();
  icp.setBackend(custom);
  ICL_TEST_EQ(icp.getBackend(), custom.get());
}
