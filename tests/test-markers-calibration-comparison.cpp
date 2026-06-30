// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Phase-B calibration comparison harness. Drives BOTH CalibrationTargets
// (checkerboard + marker-grid) and every detector backend through one common
// measurement: render a perspective VIEW of the target (warp generate() by a
// homography, + pixel noise), run detect(), then score the returned
// correspondences by (a) completeness (found / expected) and (b) the RMS
// reprojection residual of a homography fit obj(mm, z=0) -> img(px) — which is the
// self-consistent quality metric a planar calibration ultimately consumes. The
// result is an apples-to-apples table:
//   checkerboard: native-growth / native-ransac / native-graph / opencv
//   marker-grid : corner refine none / edge / pattern
// across viewpoints (frontal / keystone) and noise. It prints the table and
// asserts the clean frontal case is complete + sub-pixel for the native paths.

#include "harness/Test.h"
#include <icl/markers/CheckerboardTarget.h>
#include <icl/markers/MarkerGridTarget.h>
#include <icl/cv/NativeCheckerboardDetector.h>
#include <icl/cv/RansacCheckerboardDetector.h>
#include <icl/cv/GraphCheckerboardDetector.h>
#include <icl/cv/OpenCVCheckerboardDetector.h>
#include <icl/math/transform/Homography2D.h>
#include <icl/core/Img.h>
#include <cstdio>
#include <cmath>
#include <memory>
#include <vector>

using namespace icl;
using namespace icl::markers;
using icl::core::Img8u;
using icl::core::Channel8u;
using icl::utils::Size;
using icl::utils::Point32f;
using icl::math::Homography2D;

namespace {
  struct Rng {
    uint64_t s; explicit Rng(uint64_t seed):s(seed?seed:1){}
    uint64_t next(){ s^=s<<13; s^=s>>7; s^=s<<17; return s; }
    double uniform(){ return (next()>>11)*(1.0/9007199254740992.0); }
    double gauss(){ double u1=std::max(1e-12,uniform()),u2=uniform(); return std::sqrt(-2*std::log(u1))*std::cos(2*M_PI*u2); }
  };

  // perspective view of a frontal target image: warp gen by the homography taking
  // gen's corners to a (keystone) quad in the view, + optional gaussian noise.
  Img8u renderView(const Img8u &gen, Size view, float keystone, float noise, Rng &rng) {
    const float W = gen.getWidth(), H = gen.getHeight();
    const float cx = view.width/2.f, cy = view.height/2.f;
    const float hw = view.width*0.40f, hh = view.height*0.40f;
    const float ts = 1.f - keystone;                       // top edge shrink
    const Point32f genC[4] = {{0,0},{W,0},{W,H},{0,H}};    // TL,TR,BR,BL
    const Point32f dst[4]  = {{cx-hw*ts,cy-hh},{cx+hw*ts,cy-hh},{cx+hw,cy+hh},{cx-hw,cy+hh}};
    const Homography2D Hv2g(genC, dst, 4);                 // apply(viewPt) -> genPt
    Img8u out(view, 1); out.fill(255);
    Channel8u o = out[0]; const Channel8u g = gen[0];
    const int gw = gen.getWidth(), gh = gen.getHeight();
    for (int y = 0; y < view.height; ++y)
      for (int x = 0; x < view.width; ++x) {
        const Point32f p = Hv2g.apply(Point32f(x,y));
        if (p.x < 0 || p.y < 0 || p.x > gw-1 || p.y > gh-1) continue;   // white bg
        const int x0=(int)p.x, y0=(int)p.y, x1=std::min(x0+1,gw-1), y1=std::min(y0+1,gh-1);
        const float fx=p.x-x0, fy=p.y-y0;
        float v = (1-fx)*(1-fy)*g(x0,y0)+fx*(1-fy)*g(x1,y0)+(1-fx)*fy*g(x0,y1)+fx*fy*g(x1,y1);
        if (noise > 0) v += (float)(noise*rng.gauss());
        o(x,y) = (icl8u)std::min(255.f, std::max(0.f, v));
      }
    return out;
  }

  // RMS residual of a homography fit obj(2D mm) -> img(px) over the correspondences
  double homographyResidual(const std::vector<CalibrationCorrespondence> &c) {
    if (c.size() < 4) return 1e9;
    std::vector<Point32f> obj(c.size()), img(c.size());
    for (size_t i = 0; i < c.size(); ++i) { obj[i] = Point32f(c[i].objectPos[0], c[i].objectPos[1]); img[i] = c[i].imagePos; }
    const Homography2D H(img.data(), obj.data(), (int)obj.size());     // apply(obj) -> img
    double e = 0; for (size_t i = 0; i < obj.size(); ++i) e += std::pow(H.apply(obj[i]).distanceTo(img[i]), 2.0);
    return std::sqrt(e/obj.size());
  }

  struct Score { int found, expected; double residual; };
  Score score(const CalibrationTarget &t, const Img8u &view) {
    const auto c = t.detect(view);
    return { (int)c.size(), (int)t.modelPoints().size(), homographyResidual(c) };
  }
}

ICL_REGISTER_TEST("markers.calibration.comparison_harness",
                  "Phase-B: detector backends x targets x viewpoints -> completeness + residual table")
{
  Rng rng(12345);
  const Size VIEW(640, 480);
  struct Cond { const char *name; float keystone, noise; };
  const Cond conds[] = { {"frontal",0.0f,0.0f}, {"keystone",0.30f,0.0f}, {"noisy",0.15f,1.5f} };

  std::printf("\n[phaseB] === checkerboard (7x5 -> 6x4=24 inner corners) ===\n");
  std::printf("[phaseB] %-14s | %-18s | %-18s | %-18s\n", "backend", conds[0].name, conds[1].name, conds[2].name);
  CheckerboardTarget cb(7, 5, 25.f);
  const Img8u cbGen = cb.generate(Size(560, 420));
  struct CbBackend { const char *name; std::shared_ptr<cv::CheckerboardDetector> det; };
  std::vector<CbBackend> cbBackends = {
    {"native-growth", std::make_shared<cv::NativeCheckerboardDetector>()},
    {"native-ransac", std::make_shared<cv::RansacCheckerboardDetector>()},
    {"native-graph",  std::make_shared<cv::GraphCheckerboardDetector>()},
    {"opencv",        std::make_shared<cv::OpenCVCheckerboardDetector>()},
  };
  Score cbFrontalNative{0,1,1e9};
  for (auto &b : cbBackends) {
    cb.setDetector(b.det);
    std::printf("[phaseB] %-14s |", b.name);
    for (int ci = 0; ci < 3; ++ci) {
      const Img8u v = renderView(cbGen, VIEW, conds[ci].keystone, conds[ci].noise, rng);
      const Score s = score(cb, v);
      if (s.found < 4 || s.residual > 100) std::printf(" %2d/%2d  (fail)      |", s.found, s.expected);
      else                                 std::printf(" %2d/%2d r=%6.3f    |", s.found, s.expected, s.residual);
      if (ci == 0 && std::string(b.name) == "native-growth") cbFrontalNative = s;
    }
    std::printf("\n");
  }

  std::printf("\n[phaseB] === marker-grid (4x3 -> 12 markers = 48 corners) ===\n");
  std::printf("[phaseB] %-14s | %-18s | %-18s | %-18s\n", "refine", conds[0].name, conds[1].name, conds[2].name);
  MarkerGridTarget mg(Size(4,3), icl::utils::Size32f(20,20), icl::utils::Size32f(4*20+3*10,3*20+2*10));
  const Img8u mgGen = mg.generate(Size(560, 420));
  using RM = MarkerGridTarget::RefineMode;
  const std::pair<const char*,RM> mgModes[] = { {"none",RM::None}, {"edge",RM::Edge}, {"pattern",RM::Pattern} };
  Score mgFrontalEdge{0,1,1e9};
  for (auto &m : mgModes) {
    mg.setRefineMode(m.second);
    std::printf("[phaseB] %-14s |", m.first);
    for (int ci = 0; ci < 3; ++ci) {
      const Img8u v = renderView(mgGen, VIEW, conds[ci].keystone, conds[ci].noise, rng);
      const Score s = score(mg, v);
      if (s.found < 4 || s.residual > 100) std::printf(" %2d/%2d  (fail)      |", s.found, s.expected);
      else                                 std::printf(" %2d/%2d r=%6.3f    |", s.found, s.expected, s.residual);
      if (ci == 0 && std::string(m.first) == "edge") mgFrontalEdge = s;
    }
    std::printf("\n");
  }
  std::printf("\n");

  // sanity: the clean frontal case is fully detected + sub-pixel for the native paths
  ICL_TEST_EQ(cbFrontalNative.found, cbFrontalNative.expected);
  ICL_TEST_TRUE(cbFrontalNative.residual < 1.0);
  ICL_TEST_EQ(mgFrontalEdge.found, mgFrontalEdge.expected);
  ICL_TEST_TRUE(mgFrontalEdge.residual < 1.0);
}
