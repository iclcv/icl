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

// --- ColorNN (color-aware backend) --------------------------------------------

// The metric breaks a geometric tie by colour: a query equidistant from two
// targets picks the one whose colour matches the query's.
ICL_REGISTER_TEST("cv3d.icp.colornn_color_breaks_geometric_tie",
                  "ColorNN picks the colour-matching target when positions tie")
{
  const std::vector<Vec> target = { Vec(0,0,0,1), Vec(10,0,0,1) };
  const std::vector<GeomColor> tCol = { GeomColor(255,0,0,255),   // A red
                                        GeomColor(0,255,0,255) };  // B green
  const std::vector<Vec> query = { Vec(5,0,0,1) };                 // 5 from each

  ColorNN nn(1.0f);
  nn.setTargetColors(tCol);
  nn.build(target);
  std::vector<Vec> out;

  nn.setSourceColors({ GeomColor(0,255,0,255) });   // green -> should match B
  nn.nearest(query, out);
  ICL_TEST_NEAR(out[0][0], 10.0, 1e-5);

  nn.setSourceColors({ GeomColor(255,0,0,255) });   // red -> should match A
  nn.nearest(query, out);
  ICL_TEST_NEAR(out[0][0], 0.0, 1e-5);

  // with colour disabled the tie falls back to (first-found) position NN
  nn.setColorWeight(0.f);
  nn.nearest(query, out);
  ICL_TEST_NEAR(out[0][0], 0.0, 1e-5);              // A found first at equal dist
}

// A ColorNN with zero colour weight is a plain (exact) position NN, so it
// recovers the same known transform as the default octree backend.
ICL_REGISTER_TEST("cv3d.icp.colornn_zero_weight_recovers_transform",
                  "ColorNN(0) behaves as a position NN and recovers the transform")
{
  const std::vector<Vec> target = randomCloud(200, 42);
  const Mat4 T = icl::math::create_hom_4x4<float>(0.15f, -0.1f, 0.08f, 5, -5, 2);
  std::vector<Vec> source;
  for (const Vec &v : target) source.push_back(T * v);

  ICP icp(50, 20.0f, 1e-4);
  icp.setBackend(std::make_shared<ColorNN>(0.0f));   // no colours set
  std::vector<Vec> aligned;
  ICP::Result r = icp.apply(target, source, aligned);

  ICL_TEST_LT(r.error, 0.5);
  ICL_TEST_LT(meanResidual(aligned, target), 0.5);
}

// End-to-end color-aware ICP: consistent per-point colours (source[i] carries
// target[i]'s colour) aid correspondence and still recover the known transform.
ICL_REGISTER_TEST("cv3d.icp.color_aware_icp_recovers_transform",
                  "color-aware ICP recovers a known transform with consistent colours")
{
  const std::vector<Vec> target = randomCloud(200, 99);
  std::vector<GeomColor> colors;
  for (size_t i = 0; i < target.size(); ++i)                  // a deterministic colour per point
    colors.push_back(GeomColor(float(i % 256), float((i*37) % 256), float((i*91) % 256), 255));

  const Mat4 T = icl::math::create_hom_4x4<float>(-0.12f, 0.09f, -0.05f, -4, 6, 3);
  std::vector<Vec> source;
  for (const Vec &v : target) source.push_back(T * v);
  // source[i] is the transform of target[i], so they share colour i

  auto cnn = std::make_shared<ColorNN>(0.5f);
  cnn->setTargetColors(colors);
  cnn->setSourceColors(colors);

  ICP icp(50, 20.0f, 1e-4);
  icp.setBackend(cnn);
  std::vector<Vec> aligned;
  ICP::Result r = icp.apply(target, source, aligned);

  std::cout << "[icp] color-aware iterations=" << r.iterations
            << " error=" << r.error << std::endl;
  ICL_TEST_LT(r.error, 0.5);
  ICL_TEST_LT(meanResidual(aligned, target), 0.5);
}
