// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Growth-based checkerboard grid recovery (v1): ChESS saddle seeds -> ordered
// (col,row) lattice. Tested on clean, barrel-distorted, and perspective boards,
// since the calibration use case must survive both lens distortion and oblique
// viewpoints. The recovered lattice dimensions and completeness are checked
// against the known board geometry.

#include "harness/Test.h"
#include <icl/cv/CheckerboardSaddleDetector.h>
#include <icl/cv/CheckerboardGrid.h>
#include <icl/core/Img.h>
#include <algorithm>
#include <cmath>
#include <functional>

using namespace icl;
using namespace icl::cv;
using icl::core::Img8u;
using icl::utils::Size;
using icl::utils::Point32f;

namespace {
  // A board of cols x rows squares (size sq), CENTRED in the WxH image on a white
  // quiet zone — so exactly (cols-1) x (rows-1) interior X-junctions are saddles
  // (the board's outer edge is straight / L-junctions, not detected). `warp` maps
  // an output pixel -> (pre-distortion) image coordinate; identity = a clean board.
  Img8u renderBoard(int W, int H, int cols, int rows, float sq,
                    std::function<Point32f(float,float)> warp) {
    const float bw = cols*sq, bh = rows*sq, ox = (W-bw)/2.f, oy = (H-bh)/2.f;
    auto val = [&](float ix, float iy) -> float {
      const float lx = ix-ox, ly = iy-oy;
      if (lx < 0 || ly < 0 || lx >= bw || ly >= bh) return 255.f;   // quiet zone
      const int cx = (int)std::floor(lx/sq), cy = (int)std::floor(ly/sq);
      return ((cx+cy) & 1) ? 0.f : 255.f;
    };
    Img8u img(Size(W,H), 1);
    icl8u *d = img.begin(0);
    for (int y=0; y<H; ++y) for (int x=0; x<W; ++x) {
      float acc = 0; const int SS = 4;
      for (int sy=0; sy<SS; ++sy) for (int sx=0; sx<SS; ++sx) {
        const Point32f m = warp(x + (sx+0.5f)/SS - 0.5f, y + (sy+0.5f)/SS - 0.5f);
        acc += val(m.x, m.y);
      }
      d[y*W+x] = (icl8u)(acc/(SS*SS) + 0.5f);
    }
    return img;
  }

  // a recovered lattice is "neighbour-consistent" if every filled interior cell's
  // 4-neighbours are also filled and at a plausible spacing (no wild jumps)
  bool latticeConsistent(const CheckerboardGrid &g) {
    for (int r=0; r<g.rows; ++r) for (int c=0; c<g.cols; ++c) {
      if (!g.has(c,r)) continue;
      if (g.has(c-1,r) && g.has(c+1,r)) {
        const float dl = std::hypot(g.at(c,r).x-g.at(c-1,r).x, g.at(c,r).y-g.at(c-1,r).y);
        const float dr = std::hypot(g.at(c+1,r).x-g.at(c,r).x, g.at(c+1,r).y-g.at(c,r).y);
        if (dl < 1.f || dr < 1.f || dl/dr > 3.f || dr/dl > 3.f) return false;
      }
    }
    return true;
  }
}

// dims match (cols-1)x(rows-1) in either axis order (v1 doesn't canonicalise)
static bool dimsMatch(const CheckerboardGrid &g, int cols, int rows) {
  return (g.cols==cols-1 && g.rows==rows-1) || (g.cols==rows-1 && g.rows==cols-1);
}

// Clean board: the full (cols-1)x(rows-1) inner-corner lattice is recovered.
ICL_REGISTER_TEST("cv.checkergrid.clean_complete",
                  "clean board recovers the full inner-corner lattice")
{
  const int COLS=9, ROWS=7;                              // -> 8x6 inner corners
  Img8u img = renderBoard(480, 400, COLS, ROWS, 40, [](float x,float y){ return Point32f(x,y); });

  const auto seeds = CheckerboardSaddleDetector().detect(img);
  const CheckerboardGrid g = recoverCheckerboardGrid(seeds);

  std::cout << "[checkergrid] clean: " << g.cols << "x" << g.rows
            << " count=" << g.count << " (seeds=" << seeds.size() << ")" << std::endl;
  ICL_TEST_EQ((int)seeds.size(), (COLS-1)*(ROWS-1));    // only the interior X-junctions
  ICL_TEST_TRUE(dimsMatch(g, COLS, ROWS));
  ICL_TEST_EQ(g.count, (COLS-1)*(ROWS-1));
  ICL_TEST_TRUE(g.complete());
  ICL_TEST_TRUE(latticeConsistent(g));
}

// Square board: still a complete lattice (the diagonal-sublattice trap).
ICL_REGISTER_TEST("cv.checkergrid.square_complete",
                  "square board recovers the full lattice (not the diagonal half)")
{
  const int N=9;                                         // -> 8x8 inner corners
  Img8u img = renderBoard(420, 420, N, N, 40, [](float x,float y){ return Point32f(x,y); });

  const auto seeds = CheckerboardSaddleDetector().detect(img);
  const CheckerboardGrid g = recoverCheckerboardGrid(seeds);

  std::cout << "[checkergrid] square: " << g.cols << "x" << g.rows
            << " count=" << g.count << " (seeds=" << seeds.size() << ")" << std::endl;
  ICL_TEST_TRUE(dimsMatch(g, N, N));
  ICL_TEST_EQ(g.count, (N-1)*(N-1));
  ICL_TEST_TRUE(g.complete());
}

// Barrel distortion (bent edges): local growth still recovers the full lattice.
ICL_REGISTER_TEST("cv.checkergrid.distorted",
                  "barrel-distorted board still recovers the full lattice")
{
  const int COLS=9, ROWS=7; const int W=480, H=400; const float k=0.12f;
  const float cx=W/2.f, cy=H/2.f;
  auto warp = [&](float qx,float qy){
    const float nx=(qx-cx)/cx, ny=(qy-cy)/cy, r2=nx*nx+ny*ny, f=1+k*r2;
    return Point32f(cx+(qx-cx)*f, cy+(qy-cy)*f);
  };
  Img8u img = renderBoard(W, H, COLS, ROWS, 40, warp);

  const auto seeds = CheckerboardSaddleDetector().detect(img);
  const CheckerboardGrid g = recoverCheckerboardGrid(seeds);

  std::cout << "[checkergrid] distorted: " << g.cols << "x" << g.rows
            << " count=" << g.count << " (seeds=" << seeds.size() << ")" << std::endl;
  ICL_TEST_TRUE(dimsMatch(g, COLS, ROWS));
  ICL_TEST_EQ(g.count, (COLS-1)*(ROWS-1));
  ICL_TEST_TRUE(latticeConsistent(g));
}

// Perspective (oblique viewpoint): spacing varies across the image, exercising
// the per-link step-vector refinement.
ICL_REGISTER_TEST("cv.checkergrid.perspective",
                  "perspective-warped board recovers the full lattice")
{
  const int COLS=9, ROWS=7; const int W=480, H=400;
  const float cx=W/2.f, cy=H/2.f;
  // output pixel -> source coord: vertical foreshortening about the centre
  auto warp = [&](float qx,float qy){
    const float v = (qy-cy)/cy;                 // -1..1
    const float persp = 1.f + 0.35f*v;          // top rows compressed, bottom stretched
    return Point32f(cx + (qx-cx)*persp, qy);
  };
  Img8u img = renderBoard(W, H, COLS, ROWS, 40, warp);

  const auto seeds = CheckerboardSaddleDetector().detect(img);
  const CheckerboardGrid g = recoverCheckerboardGrid(seeds);

  std::cout << "[checkergrid] perspective: " << g.cols << "x" << g.rows
            << " count=" << g.count << " (seeds=" << seeds.size() << ")" << std::endl;
  ICL_TEST_TRUE(dimsMatch(g, COLS, ROWS));
  ICL_TEST_EQ(g.count, (COLS-1)*(ROWS-1));
  ICL_TEST_TRUE(latticeConsistent(g));
}

// Edge validation pass: real lattice edges (on a B/W square border) score high;
// a corrupted (diagonal, through-a-square) edge scores markedly lower.
ICL_REGISTER_TEST("cv.checkergrid.edge_scoring",
                  "edge confidence is high on real edges, low on a through-square edge")
{
  const int COLS=9, ROWS=7;
  Img8u img = renderBoard(480, 400, COLS, ROWS, 40, [](float x,float y){ return Point32f(x,y); });
  const auto seeds = CheckerboardSaddleDetector().detect(img);
  CheckerboardGrid g = recoverCheckerboardGrid(seeds);
  scoreCheckerboardGridEdges(g, img);
  ICL_TEST_TRUE(g.scored());

  double mn = 1e9, sum = 0; int cnt = 0;
  for (int r=0; r<g.rows; ++r) for (int c=0; c<g.cols; ++c) {
    if (g.has(c+1,r)) { const float s=g.rightScore(c,r); mn=std::min(mn,(double)s); sum+=s; ++cnt; }
    if (g.has(c,r+1)) { const float s=g.downScore(c,r);  mn=std::min(mn,(double)s); sum+=s; ++cnt; }
  }
  const double mean = sum/cnt;
  std::cout << "[checkergrid] edge scores: mean=" << mean << " min=" << mn << std::endl;
  ICL_TEST_TRUE(mean > 0.6);   // real edges sit on strong B/W borders
  ICL_TEST_TRUE(mn   > 0.3);

  // Corrupt one interior corner by ~half a cell so its edges cut across squares;
  // the affected edge's confidence must drop clearly.
  const float cell = std::hypot(g.at(1,0).x-g.at(0,0).x, g.at(1,0).y-g.at(0,0).y);
  const int cc=3, rr=3;
  const float good = g.rightScore(cc-1, rr);      // edge (cc-1,rr)-(cc,rr), real
  CheckerboardGrid bad = g;
  bad.points[(size_t)rr*bad.cols + cc].x += 0.5f*cell;
  bad.points[(size_t)rr*bad.cols + cc].y += 0.5f*cell;
  scoreCheckerboardGridEdges(bad, img);
  std::cout << "[checkergrid] corrupt edge: good=" << good
            << " bad=" << bad.rightScore(cc-1, rr) << std::endl;
  ICL_TEST_TRUE(bad.rightScore(cc-1, rr) < good - 0.2f);
}

// Rotated + keystoned view: the two board axes are NON-ORTHOGONAL in the image
// (as under a real camera tilt). This is the case where a perpendicular-guess
// second axis would wrongly link DIAGONAL neighbours; recovery must use the true
// second axis. (latticeConsistent alone wouldn't catch a diagonal lattice — the
// exact dims + full count do.)
ICL_REGISTER_TEST("cv.checkergrid.tilted_nonorthogonal",
                  "non-orthogonal (tilted) axes recover the full axis-aligned lattice")
{
  const int COLS=9, ROWS=7; const int W=520, H=440;
  const float cx=W/2.f, cy=H/2.f, ang=0.45f, ca=std::cos(ang), sa=std::sin(ang);
  auto warp = [&](float qx,float qy){            // output px -> model coord
    const float rx = ca*(qx-cx) - sa*(qy-cy) + cx;   // in-plane rotation
    const float ry = sa*(qx-cx) + ca*(qy-cy) + cy;
    const float v = (ry-cy)/cy, persp = 1.f + 0.4f*v;  // + vertical keystone
    return Point32f(cx + (rx-cx)*persp, ry);
  };
  Img8u img = renderBoard(W, H, COLS, ROWS, 38, warp);

  const auto seeds = CheckerboardSaddleDetector().detect(img);
  const CheckerboardGrid g = recoverCheckerboardGrid(seeds);

  std::cout << "[checkergrid] tilted: " << g.cols << "x" << g.rows
            << " count=" << g.count << " (seeds=" << seeds.size() << ")" << std::endl;
  ICL_TEST_TRUE(dimsMatch(g, COLS, ROWS));
  ICL_TEST_EQ(g.count, (COLS-1)*(ROWS-1));
  ICL_TEST_TRUE(latticeConsistent(g));
}
