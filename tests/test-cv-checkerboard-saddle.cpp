// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// CheckerboardSaddleDetector — the native (ChESS-style) checkerboard X-junction
// detector. Synthetic, headless, and explicitly tested on a BARREL-DISTORTED
// board (bent edges) as well as a clean one, since the calibration use case must
// survive lens distortion. Measures sub-pixel corner-localisation accuracy
// against the known ground-truth corner positions.

#include "harness/Test.h"
#include <icl/cv/CheckerboardSaddleDetector.h>
#include <icl/core/Img.h>
#include <icl/utils/time/Time.h>
#include <cmath>
#include <functional>

using namespace icl;
using namespace icl::cv;
using icl::core::Img8u;
using icl::utils::Size;
using icl::utils::Point32f;

namespace {
  // continuous, anti-aliased checkerboard value at a MODEL coordinate (mm≈px)
  inline float checkerVal(float mx, float my, float sq) {
    const int cx = (int)std::floor(mx/sq), cy = (int)std::floor(my/sq);
    return ((cx+cy) & 1) ? 255.f : 0.f;
  }

  // render a board; `warp` maps an output pixel → model coordinate (identity for
  // a clean board, a barrel map for a distorted one). 4×4 supersampling → smooth
  // edges so the saddle is sub-pixel meaningful.
  Img8u renderBoard(int W, int H, float sq, std::function<Point32f(float,float)> warp) {
    Img8u img(Size(W,H), 1);
    icl8u *d = img.begin(0);
    for (int y=0; y<H; ++y) for (int x=0; x<W; ++x) {
      float acc = 0; const int SS = 4;
      for (int sy=0; sy<SS; ++sy) for (int sx=0; sx<SS; ++sx) {
        // pixel (x,y) data lives at integer position (x,y) (sampler convention),
        // so it must represent the model AREA centred on (x,y): offset by -0.5.
        const Point32f m = warp(x + (sx+0.5f)/SS - 0.5f, y + (sy+0.5f)/SS - 0.5f);
        acc += checkerVal(m.x, m.y, sq);
      }
      d[y*W+x] = (icl8u)(acc/(SS*SS) + 0.5f);
    }
    return img;
  }

  // a representative board (warp = identity unless given) for A/B + benchmark
  Img8u benchBoard(int W=1000, int H=1000, float sq=40) {
    return [&]{
      const float cx=W/2.f, cy=H/2.f, k=0.08f;
      return renderBoard(W, H, sq, [&](float qx,float qy){
        const float nx=(qx-cx)/cx, ny=(qy-cy)/cy, r2=nx*nx+ny*ny, f=1+k*r2;
        return Point32f(cx+(qx-cx)*f, cy+(qy-cy)*f);
      });
    }();
  }

  // nearest detected seed to p; returns distance (or 1e9 if none)
  float nearest(const std::vector<CornerSeed> &s, const Point32f &p) {
    float best = 1e9f;
    for (const auto &c : s) {
      const float dx=c.pos.x-p.x, dy=c.pos.y-p.y, dd=std::sqrt(dx*dx+dy*dy);
      if (dd < best) best = dd;
    }
    return best;
  }
}

// Clean board: every internal corner is detected to sub-pixel accuracy.
ICL_REGISTER_TEST("cv.checkersaddle.clean_subpixel",
                  "all internal corners detected; mean localisation error < 0.5 px")
{
  const int W=360, H=360; const float sq=40;
  Img8u img = renderBoard(W, H, sq, [](float x,float y){ return Point32f(x,y); });

  CheckerboardSaddleDetector det;
  const std::vector<CornerSeed> seeds = det.detect(img);

  int found=0, total=0; double sumErr=0, maxErr=0;
  for (int j=1; j<H/(int)sq; ++j) for (int i=1; i<W/(int)sq; ++i) {
    const Point32f corner(i*sq, j*sq);
    ++total;
    const float e = nearest(seeds, corner);
    if (e < 1.5f) { ++found; sumErr += e; if (e>maxErr) maxErr=e; }
  }
  const double meanErr = found ? sumErr/found : 1e9;
  std::cout << "[checkersaddle] clean: found " << found << "/" << total
            << "  meanErr=" << meanErr << "px maxErr=" << maxErr << "px  ("
            << seeds.size() << " seeds)" << std::endl;
  ICL_TEST_EQ(found, total);            // every internal corner found
  ICL_TEST_EQ(meanErr < 0.5, true);     // sub-pixel accuracy
}

// The optimized dense-response path must match the reference path: same corner
// count, same sub-pixel positions. It is NOT bit-identical — and the optimized
// path is in fact slightly MORE accurate: the reference computes the bilinear
// fraction as (x+dx) - floor(x+dx), which loses ~14 mantissa bits at large x
// (~6e-5 at x≈645); the optimized path uses dx - floor(dx) at full precision.
// So we assert a tight tolerance, not exact equality.
ICL_REGISTER_TEST("cv.checkersaddle.optimized_matches_reference",
                  "optimized path matches the reference path (same corners, sub-pixel agreement)")
{
  const Img8u img = benchBoard();

  auto run = [&](bool opt){
    CheckerboardSaddleDetector::Params p; p.optimized = opt;
    return CheckerboardSaddleDetector(p).detect(img);
  };
  const std::vector<CornerSeed> ref = run(false);
  const std::vector<CornerSeed> fast = run(true);

  ICL_TEST_TRUE(!ref.empty());
  ICL_TEST_EQ(fast.size(), ref.size());
  double maxPos = 0, maxField = 0;
  for (size_t i = 0; i < std::min(ref.size(), fast.size()); ++i) {
    maxPos   = std::max(maxPos,   (double)std::fabs(fast[i].pos.x - ref[i].pos.x));
    maxPos   = std::max(maxPos,   (double)std::fabs(fast[i].pos.y - ref[i].pos.y));
    maxField = std::max({maxField, (double)std::fabs(fast[i].score - ref[i].score),
                                   (double)std::fabs(fast[i].orientation - ref[i].orientation)});
  }
  std::cout << "[checkersaddle] opt-vs-ref: " << ref.size()
            << " seeds, max pos diff=" << maxPos << "px, max score/ori diff=" << maxField << std::endl;
  ICL_TEST_EQ(maxPos < 1e-3, true);     // sub-pixel positions agree to <0.001 px
  ICL_TEST_EQ(maxField < 1e-3, true);   // scores/orientations agree to <0.001
}

// Benchmark: reference vs optimized detect() on a 640x480 board.
ICL_REGISTER_TEST("cv.checkersaddle.benchmark",
                  "report reference vs optimized detect() timing")
{
  const Img8u img = benchBoard();
  const int N = 30;

  auto bench = [&](bool opt){
    CheckerboardSaddleDetector::Params p; p.optimized = opt;
    CheckerboardSaddleDetector det(p);
    det.detect(img);                                   // warm up
    const utils::Time t0 = utils::Time::now();
    for (int i = 0; i < N; ++i) det.detect(img);
    return (utils::Time::now() - t0).toMicroSeconds() / 1000.0 / N;   // ms/call
  };
  const double refMs  = bench(false);
  const double fastMs = bench(true);
  std::cout << "[checkersaddle] benchmark " << img.getWidth() << "x" << img.getHeight()
            << "  reference=" << refMs << " ms  optimized=" << fastMs
            << " ms  speedup=" << (refMs/fastMs) << "x" << std::endl;

  // RGB input exercises the toGray() colour-conversion path (the gray fast-path
  // above is a plain cast). Reports its extra cost over the gray detect.
  Img8u rgb(img.getSize(), 3);
  for (int ch = 0; ch < 3; ++ch)
    std::copy(img.begin(0), img.begin(0)+img.getDim(), rgb.begin(ch));
  CheckerboardSaddleDetector detRGB;   // optimized default
  detRGB.detect(rgb);
  const utils::Time t0 = utils::Time::now();
  for (int i = 0; i < N; ++i) detRGB.detect(rgb);
  const double rgbMs = (utils::Time::now() - t0).toMicroSeconds() / 1000.0 / N;
  std::cout << "[checkersaddle] optimized RGB-input=" << rgbMs << " ms  (toGray ~"
            << (rgbMs - fastMs) << " ms)" << std::endl;

  ICL_TEST_TRUE(fastMs > 0.0);   // informational; timing printed above
}

// Barrel-distorted board (bent edges): the local saddle detector still finds the
// corners accurately — the property that matters for lens-distorted calibration.
ICL_REGISTER_TEST("cv.checkersaddle.distorted_robust",
                  "corners survive barrel distortion (bent edges); most found, accurate")
{
  const int W=360, H=360; const float sq=40, k=0.10f;
  const float cx=W/2.f, cy=H/2.f;
  auto warp = [&](float qx,float qy){           // output pixel → model coord
    const float nx=(qx-cx)/cx, ny=(qy-cy)/cy, r2=nx*nx+ny*ny, f=1+k*r2;
    return Point32f(cx+(qx-cx)*f, cy+(qy-cy)*f);
  };
  Img8u img = renderBoard(W, H, sq, warp);

  CheckerboardSaddleDetector det;
  const std::vector<CornerSeed> seeds = det.detect(img);

  // ground-truth image position of a model corner g: fixed-point invert warp
  auto cornerImagePos = [&](const Point32f &g){
    Point32f q = g;
    for (int it=0; it<12; ++it) { Point32f w=warp(q.x,q.y); q.x += g.x-w.x; q.y += g.y-w.y; }
    return q;
  };

  int found=0, total=0; double sumErr=0;
  for (int j=1; j<H/(int)sq; ++j) for (int i=1; i<W/(int)sq; ++i) {
    const Point32f q = cornerImagePos(Point32f(i*sq, j*sq));
    if (q.x<20||q.y<20||q.x>W-20||q.y>H-20) continue;   // skip near-edge
    ++total;
    const float e = nearest(seeds, q);
    if (e < 1.5f) { ++found; sumErr += e; }
  }
  const double meanErr = found ? sumErr/found : 1e9;
  std::cout << "[checkersaddle] distorted: found " << found << "/" << total
            << "  meanErr=" << meanErr << "px  (" << seeds.size() << " seeds)" << std::endl;
  ICL_TEST_EQ(found >= total*0.9, true);   // ≥90% of corners survive the distortion
  ICL_TEST_EQ(meanErr < 0.7, true);        // and stay accurate
}
