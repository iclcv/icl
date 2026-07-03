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

  // The two quads defining a keystone view: the frontal gen-image corners and the
  // (foreshortened) quad they map to in the view. Shared by renderView (which warps
  // by their homography) and the ground-truth mapping (its inverse), so the pixels
  // and the analytic corner positions can never drift apart.
  void viewQuad(const Img8u &gen, Size view, float keystone, Point32f genC[4], Point32f dst[4]) {
    const float W = gen.getWidth(), H = gen.getHeight();
    const float cx = view.width/2.f, cy = view.height/2.f;
    const float hw = view.width*0.40f, hh = view.height*0.40f;
    const float ts = 1.f - keystone;                       // top edge shrink
    genC[0]={0,0}; genC[1]={W,0}; genC[2]={W,H}; genC[3]={0,H};   // TL,TR,BR,BL
    dst[0]={cx-hw*ts,cy-hh}; dst[1]={cx+hw*ts,cy-hh}; dst[2]={cx+hw,cy+hh}; dst[3]={cx-hw,cy+hh};
  }

  // gen-image -> view homography (the INVERSE of the warp renderView applies): maps
  // an analytically-known gen corner to its exact position in the rendered view.
  Homography2D genToView(const Img8u &gen, Size view, float keystone) {
    Point32f genC[4], dst[4]; viewQuad(gen, view, keystone, genC, dst);
    return Homography2D::fit(genC, dst, 4);                     // apply(genPt) -> viewPt
  }

  // perspective view of a frontal target image: warp gen by the homography taking
  // gen's corners to a (keystone) quad in the view, + optional gaussian noise.
  Img8u renderView(const Img8u &gen, Size view, float keystone, float noise, Rng &rng) {
    Point32f genC[4], dst[4]; viewQuad(gen, view, keystone, genC, dst);
    const Homography2D Hv2g = Homography2D::fit(dst, genC, 4);                 // apply(viewPt) -> genPt
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
    const Homography2D H = Homography2D::fit(obj.data(), img.data(), (int)obj.size());     // apply(obj) -> img
    double e = 0; for (size_t i = 0; i < obj.size(); ++i) e += std::pow(H.apply(obj[i]).distanceTo(img[i]), 2.0);
    return std::sqrt(e/obj.size());
  }

  struct Score { int found, expected; double residual; };
  Score score(const CalibrationTarget &t, const Img8u &view) {
    const auto c = t.detect(view);
    return { (int)c.size(), (int)t.modelPoints().size(), homographyResidual(c) };
  }

  // The analytically-true view positions of ALL inner corners: each gen-image
  // corner (CheckerboardTarget::generate()'s EXACT layout — px/square =
  // min(W/(cols+2), H/(rows+2)), board centred, inner corner (mc,mr) on grid line
  // (mc+1, mr+1)) mapped through the exact gen->view homography.
  std::vector<Point32f> checkerTrueCorners(const Img8u &gen, Size view, float keystone,
                                           int cols, int rows) {
    const Homography2D Hg2v = genToView(gen, view, keystone);
    const float W = gen.getWidth(), H = gen.getHeight();
    const float px = std::min(W/float(cols+2), H/float(rows+2));
    const float ox = (W - px*cols)/2.f, oy = (H - px*rows)/2.f;
    std::vector<Point32f> truth;
    for (int mr = 0; mr < rows-1; ++mr)
      for (int mc = 0; mc < cols-1; ++mc)
        truth.push_back(Hg2v.apply(Point32f(ox + (mc+1)*px, oy + (mr+1)*px)));
    return truth;
  }

  // RMS of the ABSOLUTE ground-truth corner error: distance from each detected
  // corner to its NEAREST true corner. Unlike the homography residual (self-
  // consistency) this measures real localisation accuracy; nearest-match makes it
  // invariant to the arbitrary recovered board frame (native does not canonicalise
  // the (col,row) origin/orientation, so a per-label comparison would be meaningless).
  double checkerGroundTruthRMS(const std::vector<CalibrationCorrespondence> &c,
                               const std::vector<Point32f> &truth) {
    if (c.empty()) return 1e9;
    double e = 0;
    for (const auto &cc : c) {
      float best = 1e30f;
      for (const auto &t : truth) best = std::min(best, (float)cc.imagePos.distanceTo(t));
      e += (double)best*best;
    }
    return std::sqrt(e/c.size());
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
  std::printf("[phaseB] r = homography residual (self-consistency), g = ground-truth corner RMS [px]\n");
  std::printf("[phaseB] %-14s | %-22s | %-22s | %-22s\n", "backend", conds[0].name, conds[1].name, conds[2].name);
  const int CB_C=7, CB_R=5; const float CB_SQ=25.f;   // must match `cb` below
  CheckerboardTarget cb(CB_C, CB_R, CB_SQ);
  const Img8u cbGen = cb.generate(Size(560, 420));
  struct CbBackend { const char *name; std::shared_ptr<cv::CheckerboardDetector> det; };
  std::vector<CbBackend> cbBackends = {
    {"native-growth", std::make_shared<cv::NativeCheckerboardDetector>()},
    {"native-ransac", std::make_shared<cv::RansacCheckerboardDetector>()},
    {"native-graph",  std::make_shared<cv::GraphCheckerboardDetector>()},
    {"opencv",        std::make_shared<cv::OpenCVCheckerboardDetector>()},
  };
  Score cbFrontalNative{0,1,1e9};
  double cbGtNativeFrontal=1e9, cbGtOpencvFrontal=1e9;
  for (auto &b : cbBackends) {
    cb.setDetector(b.det);
    std::printf("[phaseB] %-14s |", b.name);
    for (int ci = 0; ci < 3; ++ci) {
      const Img8u v = renderView(cbGen, VIEW, conds[ci].keystone, conds[ci].noise, rng);
      const auto c = cb.detect(v);
      const int found=(int)c.size(), expected=(int)cb.modelPoints().size();
      const double resid = homographyResidual(c);
      const double gt = checkerGroundTruthRMS(c, checkerTrueCorners(cbGen, VIEW, conds[ci].keystone, CB_C, CB_R));
      if (found < 4 || resid > 100) std::printf(" %2d/%2d (fail)           |", found, expected);
      else                          std::printf(" %2d/%2d r=%.3f g=%.3f |", found, expected, resid, gt);
      if (ci == 0) {
        if (std::string(b.name)=="native-growth") { cbFrontalNative={found,expected,resid}; cbGtNativeFrontal=gt; }
        if (std::string(b.name)=="opencv") cbGtOpencvFrontal=gt;
      }
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

  // GROUND TRUTH: the native sub-pixel polish lands corners sub-0.1px against the
  // analytically-known positions, on par with (here slightly better than) OpenCV's
  // cornerSubPix — substantiating that the accuracy gap is genuinely closed, not
  // just self-consistent. (Generous margin vs opencv: the ~0.01px lead is within
  // render/tuning noise; the real claim is "not worse".)
  std::printf("[phaseB] ground-truth frontal RMS: native=%.4f opencv=%.4f px\n",
              cbGtNativeFrontal, cbGtOpencvFrontal);
  ICL_TEST_TRUE(cbGtNativeFrontal < 0.1);
  ICL_TEST_TRUE(cbGtNativeFrontal < 1.5*cbGtOpencvFrontal);
}
