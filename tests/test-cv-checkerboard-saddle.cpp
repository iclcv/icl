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

// Multithreading (OpenMP) only parallelizes independent writes, so single- and
// multi-threaded runs must produce BIT-IDENTICAL seeds.
ICL_REGISTER_TEST("cv.checkersaddle.multithreaded_matches_singlethreaded",
                  "single- and multi-threaded detect() produce identical seeds")
{
  const Img8u img = benchBoard();

  auto run = [&](bool mt){
    CheckerboardSaddleDetector::Params p; p.multithreaded = mt;
    return CheckerboardSaddleDetector(p).detect(img);
  };
  const std::vector<CornerSeed> st = run(false);
  const std::vector<CornerSeed> mt = run(true);

  ICL_TEST_TRUE(!st.empty());
  ICL_TEST_EQ(mt.size(), st.size());
  double maxDiff = 0;
  for (size_t i = 0; i < std::min(st.size(), mt.size()); ++i)
    maxDiff = std::max({maxDiff, (double)std::fabs(mt[i].pos.x - st[i].pos.x),
                                 (double)std::fabs(mt[i].pos.y - st[i].pos.y),
                                 (double)std::fabs(mt[i].score - st[i].score),
                                 (double)std::fabs(mt[i].orientation - st[i].orientation)});
  std::cout << "[checkersaddle] st-vs-mt: " << st.size() << " seeds, max diff=" << maxDiff << std::endl;
  ICL_TEST_EQ(maxDiff, 0.0);   // bit-identical
}

// Benchmark: single-threaded vs OpenMP-parallel detect() on a 1000x1000 board.
ICL_REGISTER_TEST("cv.checkersaddle.benchmark",
                  "report single-threaded vs multithreaded detect() timing")
{
  const Img8u img = benchBoard();
  const int N = 30;

  auto bench = [&](bool mt){
    CheckerboardSaddleDetector::Params p; p.multithreaded = mt;
    CheckerboardSaddleDetector det(p);
    det.detect(img);                                   // warm up
    const utils::Time t0 = utils::Time::now();
    for (int i = 0; i < N; ++i) det.detect(img);
    return (utils::Time::now() - t0).toMicroSeconds() / 1000.0 / N;   // ms/call
  };
  const double stMs = bench(false);
  const double mtMs = bench(true);
  std::cout << "[checkersaddle] benchmark " << img.getWidth() << "x" << img.getHeight()
            << "  single-thread=" << stMs << " ms  multithread=" << mtMs
            << " ms  speedup=" << (stMs/mtMs) << "x" << std::endl;

  // RGB input exercises the toGray() colour-conversion path (the gray fast-path
  // is a plain cast). Reports its extra cost over the gray detect.
  Img8u rgb(img.getSize(), 3);
  for (int ch = 0; ch < 3; ++ch)
    std::copy(img.begin(0), img.begin(0)+img.getDim(), rgb.begin(ch));
  CheckerboardSaddleDetector detRGB;
  detRGB.detect(rgb);
  const utils::Time t0 = utils::Time::now();
  for (int i = 0; i < N; ++i) detRGB.detect(rgb);
  const double rgbMs = (utils::Time::now() - t0).toMicroSeconds() / 1000.0 / N;
  std::cout << "[checkersaddle] RGB-input=" << rgbMs << " ms  (toGray ~"
            << (rgbMs - mtMs) << " ms)" << std::endl;

  // Isolate NMS+sub-pixel: responseImage() does toGray+fillResponse only, so
  // (detect - responseImage) approximates the serial NMS + sub-pixel cost.
  {
    CheckerboardSaddleDetector det;   // multithreaded default
    det.responseImage(img);
    const utils::Time tr = utils::Time::now();
    for (int i = 0; i < N; ++i) det.responseImage(img);
    const double respMs = (utils::Time::now() - tr).toMicroSeconds() / 1000.0 / N;
    std::cout << "[checkersaddle] response-only=" << respMs << " ms  (NMS+subpixel ~"
              << (mtMs - respMs) << " ms of " << mtMs << " ms)" << std::endl;
  }

  ICL_TEST_TRUE(mtMs > 0.0);   // informational; timing printed above
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
