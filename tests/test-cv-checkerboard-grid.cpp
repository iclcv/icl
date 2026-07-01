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
#include <icl/cv/OpenCVCheckerboardDetector.h>
#include <icl/cv/RansacCheckerboardDetector.h>
#include <icl/cv/GraphCheckerboardDetector.h>
#include <icl/math/transform/Homography2D.h>
#include <icl/core/Img.h>
#include <algorithm>
#include <cmath>
#include <functional>

using namespace icl;
using namespace icl::cv;
using icl::core::Img8u;
using icl::utils::Size;
using icl::utils::Point32f;
using icl::cv::CornerSeed;

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

// Cleanup pass (refineCheckerboardGrid): a phantom partial border column —
// the documented "spurious border detections inflate the lattice dims" failure
// — is suppressed by the homography + Hungarian re-association + boundary trim,
// while a clean grid is left untouched.
ICL_REGISTER_TEST("cv.checkergrid.refine_suppresses_phantom",
                  "homography+Hungarian cleanup trims a phantom border column")
{
  const int COLS=9, ROWS=7;                               // -> 8x6 inner corners
  Img8u img = renderBoard(480, 400, COLS, ROWS, 40, [](float x,float y){ return Point32f(x,y); });
  const auto seeds = CheckerboardSaddleDetector().detect(img);
  const CheckerboardGrid g0 = recoverCheckerboardGrid(seeds);   // clean 8x6
  ICL_TEST_TRUE(dimsMatch(g0, COLS, ROWS));
  ICL_TEST_TRUE(g0.complete());

  // Build an INFLATED grid: append a phantom column (g0.cols) that is on the
  // extended lattice geometrically, but only partially detected (2 of g0.rows
  // cells) — exactly what a few spurious border seeds produce.
  const int C = g0.cols, R = g0.rows, NC = C+1;
  CheckerboardGrid bad;
  bad.cols = NC; bad.rows = R;
  bad.points.assign((size_t)NC*R, Point32f(0,0));
  bad.filled.assign((size_t)NC*R, 0);
  std::vector<CornerSeed> pool;
  for (int r=0;r<R;++r) for (int c=0;c<C;++c) {            // real corners
    const Point32f p = g0.at(c,r);
    bad.points[(size_t)r*NC+c] = p; bad.filled[(size_t)r*NC+c] = 1;
    pool.push_back({p, 1.f, 0.f});
  }
  for (int r=2;r<=3;++r) {                                  // 2 phantom corners
    const Point32f e = Point32f(g0.at(C-1,r).x-g0.at(C-2,r).x, g0.at(C-1,r).y-g0.at(C-2,r).y);
    const Point32f ph(g0.at(C-1,r).x+e.x, g0.at(C-1,r).y+e.y);
    bad.points[(size_t)r*NC+C] = ph; bad.filled[(size_t)r*NC+C] = 1;
    pool.push_back({ph, 1.f, 0.f});
  }
  bad.count = 0; for (char f : bad.filled) bad.count += f;
  ICL_TEST_EQ(bad.cols, COLS);                             // inflated to 9 columns
  ICL_TEST_EQ(bad.count, (COLS-1)*(ROWS-1) + 2);

  const CheckerboardGrid fixed = refineCheckerboardGrid(bad, pool, &img);
  std::cout << "[checkergrid] refine: " << bad.cols << "x" << bad.rows
            << " (count " << bad.count << ") -> " << fixed.cols << "x" << fixed.rows
            << " (count " << fixed.count << ")" << std::endl;
  ICL_TEST_TRUE(dimsMatch(fixed, COLS, ROWS));             // phantom column trimmed
  ICL_TEST_EQ(fixed.count, (COLS-1)*(ROWS-1));
  ICL_TEST_TRUE(fixed.complete());
  ICL_TEST_TRUE(latticeConsistent(fixed));
}

// Cleanup must be a no-op on an already-clean grid (dims + count preserved, the
// same corner positions snap back through the assignment).
ICL_REGISTER_TEST("cv.checkergrid.refine_clean_noop",
                  "cleanup leaves a clean lattice unchanged")
{
  const int COLS=9, ROWS=7;
  Img8u img = renderBoard(480, 400, COLS, ROWS, 40, [](float x,float y){ return Point32f(x,y); });
  const auto seeds = CheckerboardSaddleDetector().detect(img);
  const CheckerboardGrid g0 = recoverCheckerboardGrid(seeds);
  const CheckerboardGrid g1 = refineCheckerboardGrid(g0, seeds, &img);
  std::cout << "[checkergrid] refine_clean: " << g1.cols << "x" << g1.rows
            << " count=" << g1.count << std::endl;
  ICL_TEST_TRUE(dimsMatch(g1, COLS, ROWS));
  ICL_TEST_EQ(g1.count, (COLS-1)*(ROWS-1));
  ICL_TEST_TRUE(g1.complete());
}

// OpenCV backend (OpenCVCheckerboardDetector) detects a clean board into a
// complete grid via the CheckerboardDetector interface. (Also guards the
// inherited img_to_mat null-Mat bug that the lab surfaced.)
ICL_REGISTER_TEST("cv.opencvcheckerboard.detect",
                  "OpenCV findChessboardCorners backend yields a complete grid")
{
  const int COLS=9, ROWS=7;                              // -> 8x6 inner corners
  Img8u img = renderBoard(480, 400, COLS, ROWS, 40, [](float x,float y){ return Point32f(x,y); });

  OpenCVCheckerboardDetector det;
  CheckerboardDetector::Hints h;
  h.boardCells = Size(COLS-1, ROWS-1);                   // OpenCV needs the inner-corner dims
  const auto res = det.detect(img, h);

  std::cout << "[opencvcheckerboard] boards=" << res.boards.size()
            << (res.boards.empty() ? "" : (" " + std::to_string(res.boards.front().cols) + "x"
                                              + std::to_string(res.boards.front().rows))) << std::endl;
  ICL_TEST_EQ((int)res.boards.size(), 1);
  ICL_TEST_TRUE(dimsMatch(res.boards.front(), COLS, ROWS));
  ICL_TEST_TRUE(res.boards.front().complete());
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

// Guided growth (image edge evidence) must recover the lattice at least as well
// as geometry on a clean tilted view — and its grown edges all land on real
// black/white borders (high confidence). Guided is what disambiguates the harder
// strongly-foreshortened / false-positive cases (validated live in the lab; a
// clean synthetic for the extreme diagonal-trap is deferred with the
// false-positive-suppression work).
ICL_REGISTER_TEST("cv.checkergrid.guided_tilted",
                  "guided growth recovers the tilted lattice with high-confidence edges")
{
  const int COLS=9, ROWS=7; const int W=520, H=440;
  const float cx=W/2.f, cy=H/2.f, ang=0.45f, ca=std::cos(ang), sa=std::sin(ang);
  auto warp = [&](float qx,float qy){
    const float rx = ca*(qx-cx) - sa*(qy-cy) + cx;
    const float ry = sa*(qx-cx) + ca*(qy-cy) + cy;
    const float v = (ry-cy)/cy, persp = 1.f + 0.4f*v;
    return Point32f(cx + (rx-cx)*persp, ry);
  };
  Img8u img = renderBoard(W, H, COLS, ROWS, 38, warp);
  const auto seeds = CheckerboardSaddleDetector().detect(img);

  CheckerboardGrid g = recoverCheckerboardGrid(seeds, &img);   // guided
  ICL_TEST_TRUE(dimsMatch(g, COLS, ROWS));
  ICL_TEST_EQ(g.count, (COLS-1)*(ROWS-1));

  // every grown edge sits on a real border → high mean confidence
  scoreCheckerboardGridEdges(g, img);
  double sum=0; int cnt=0;
  for (int r=0;r<g.rows;++r) for (int c=0;c<g.cols;++c) {
    if (g.has(c+1,r)) { sum+=g.rightScore(c,r); ++cnt; }
    if (g.has(c,r+1)) { sum+=g.downScore(c,r);  ++cnt; }
  }
  std::cout << "[checkergrid] guided_tilted: " << g.cols << "x" << g.rows
            << " meanEdge=" << (sum/cnt) << std::endl;
  ICL_TEST_TRUE(sum/cnt > 0.6);
}

// --- Global RANSAC association (recoverCheckerboardGridRansac / RansacCheckerboardDetector) ---

// count/dims match for a DIRECT cols x rows inner-corner lattice (either order)
static bool fullGrid(const CheckerboardGrid &g, int cols, int rows) {
  return g.count == cols*rows &&
         ((g.cols==cols && g.rows==rows) || (g.cols==rows && g.rows==cols));
}

// The RANSAC backend recovers the full inner-corner lattice on a clean board.
ICL_REGISTER_TEST("cv.checkergrid.ransac_clean",
                  "RANSAC backend recovers the full lattice on a clean board")
{
  const int COLS=9, ROWS=7;                              // -> 8x6 inner corners
  Img8u img = renderBoard(480, 400, COLS, ROWS, 40, [](float x,float y){ return Point32f(x,y); });

  RansacCheckerboardDetector det;
  ICL_TEST_EQ(det.name(), std::string("native-ransac"));
  const auto res = det.detect(img);
  ICL_TEST_TRUE(!res.empty());
  const CheckerboardGrid &g = res.boards[0];
  std::cout << "[checkergrid] ransac_clean: " << g.cols << "x" << g.rows
            << " count=" << g.count << std::endl;
  ICL_TEST_TRUE(fullGrid(g, COLS-1, ROWS-1));
  ICL_TEST_TRUE(latticeConsistent(g));
}

// The diagonal trap: a sheared board whose cell DIAGONAL is shorter than its
// axes (steep oblique). Nearest-neighbour growth bootstraps onto the diagonal and
// collapses to a sub-lattice; the global RANSAC associator recovers the full grid.
// Image-free (seeds built directly from a sheared affine) -> deterministic.
ICL_REGISTER_TEST("cv.checkergrid.ransac_diagonal_trap",
                  "RANSAC recovers the full lattice where growth falls into the diagonal trap")
{
  const int C=9, R=6; const float L=40.f;
  auto makeSeeds = [&](float phiDeg){
    const float phi = phiDeg*(float)M_PI/180.f;
    const Point32f a(L,0.f), b(L*std::cos(phi), L*std::sin(phi));   // equal-length axes at angle phi
    std::vector<CornerSeed> s;
    for (int r=0;r<R;++r) for (int c=0;c<C;++c) {
      CornerSeed cs;
      cs.pos = Point32f(240 + c*a.x + r*b.x, 160 + c*a.y + r*b.y);
      // boost an INTERIOR seed so growth starts there (where the short diagonal
      // neighbour exists and the trap triggers) — not at a grid corner (where the
      // short-direction diagonal is off-grid, so growth would pick true axes).
      cs.score = (c==C/2 && r==R/2) ? 1.0f : 0.5f;
      s.push_back(cs);
    }
    return s;
  };

  // trap region: phi < 60 deg => |a-b| = 2L*sin(phi/2) < L = axis length
  for (float phi : {55.f, 45.f, 35.f}) {
    const auto seeds = makeSeeds(phi);
    const CheckerboardGrid gr = recoverCheckerboardGridRansac(seeds);
    std::cout << "[checkergrid] ransac_trap phi=" << phi << ": "
              << gr.cols << "x" << gr.rows << " count=" << gr.count << std::endl;
    ICL_TEST_TRUE(fullGrid(gr, C, R));
  }

  // lock the contrast: pure-geometry growth IS trapped at phi=45 (does not
  // recover the full 9x6 grid) — this is the failure the RANSAC backend fixes.
  const CheckerboardGrid gg = recoverCheckerboardGrid(makeSeeds(45.f));
  ICL_TEST_TRUE(!fullGrid(gg, C, R));
}

// Perspective robustness: a strong keystone (top edge squeezed) is exactly a
// homography. The RANSAC seed feeds the per-cell local-step growth, which tracks
// perspective, so the full grid is recovered. Image-free (seeds via a homography
// quad) -> deterministic. Guards the perspective gap the global-homography ICP
// prototype had (it failed at this keystone strength).
ICL_REGISTER_TEST("cv.checkergrid.ransac_keystone",
                  "RANSAC recovers the full lattice under strong keystone perspective")
{
  const int C=9, R=6;
  const float k=0.30f, W=400, Hh=300, cx=240, cy=160;
  const float topHalf=0.5f*W*(1.f-k), botHalf=0.5f*W;
  Point32f srcQ[4]={{0,0},{(float)(C-1),0},{(float)(C-1),(float)(R-1)},{0,(float)(R-1)}};
  Point32f dstQ[4]={{cx-topHalf,cy-0.5f*Hh},{cx+topHalf,cy-0.5f*Hh},
                    {cx+botHalf,cy+0.5f*Hh},{cx-botHalf,cy+0.5f*Hh}};
  icl::math::Homography2D H(dstQ, srcQ, 4);                  // apply(model-quad)=image-quad
  std::vector<CornerSeed> seeds;
  for (int r=0;r<R;++r) for (int c=0;c<C;++c) {
    CornerSeed cs; cs.pos = H.apply(Point32f((float)c,(float)r)); cs.score = 0.5f; seeds.push_back(cs);
  }
  const CheckerboardGrid g = recoverCheckerboardGridRansac(seeds);
  std::cout << "[checkergrid] ransac_keystone k=" << k << ": "
            << g.cols << "x" << g.rows << " count=" << g.count << std::endl;
  ICL_TEST_TRUE(fullGrid(g, C, R));
}

// --- Graph-topology association (recoverCheckerboardGridGraph / GraphCheckerboardDetector) ---
// ROCHADE-style: Delaunay adjacency pruned to grid edges by image edge evidence,
// then BFS coordinate assignment. Unlike the RANSAC tests these MUST run on a
// rendered image (the edge evidence is what removes the cell diagonals).

// Clean board: the graph backend recovers the full inner-corner lattice.
ICL_REGISTER_TEST("cv.checkergrid.graph_clean",
                  "graph backend recovers the full lattice on a clean board")
{
  const int COLS=9, ROWS=7;                              // -> 8x6 inner corners
  Img8u img = renderBoard(480, 400, COLS, ROWS, 40, [](float x,float y){ return Point32f(x,y); });

  GraphCheckerboardDetector det;
  ICL_TEST_EQ(det.name(), std::string("native-graph"));
  const auto res = det.detect(img);
  ICL_TEST_TRUE(!res.empty());
  const CheckerboardGrid &g = res.boards[0];
  std::cout << "[checkergrid] graph_clean: " << g.cols << "x" << g.rows
            << " count=" << g.count << std::endl;
  ICL_TEST_TRUE(fullGrid(g, COLS-1, ROWS-1));
  ICL_TEST_TRUE(latticeConsistent(g));
}

// Perspective (oblique keystone): the per-node local-axis re-estimation tracks
// the varying spacing, so the full lattice is recovered.
ICL_REGISTER_TEST("cv.checkergrid.graph_perspective",
                  "graph backend recovers the full lattice under perspective")
{
  const int COLS=9, ROWS=7; const int W=480, H=400;
  const float cx=W/2.f, cy=H/2.f;
  auto warp = [&](float qx,float qy){
    const float v = (qy-cy)/cy, persp = 1.f + 0.35f*v;
    return Point32f(cx + (qx-cx)*persp, qy);
  };
  Img8u img = renderBoard(W, H, COLS, ROWS, 40, warp);

  const CheckerboardGrid g = recoverCheckerboardGridGraph(
    CheckerboardSaddleDetector().detect(img), img);
  std::cout << "[checkergrid] graph_perspective: " << g.cols << "x" << g.rows
            << " count=" << g.count << std::endl;
  ICL_TEST_TRUE(fullGrid(g, COLS-1, ROWS-1));
  ICL_TEST_TRUE(latticeConsistent(g));
}

// Strong oblique shear on a RENDERED board: equal-length grid axes only 45 deg
// apart, so the cell diagonal is shorter than the axes (the diagonal-trap regime).
// The graph backend prunes the cell diagonals by image edge evidence and assigns
// coordinates by argmax-per-neighbour, so it recovers the full grid where a loose
// angle/length heuristic would walk a diagonal staircase off the row ends.
ICL_REGISTER_TEST("cv.checkergrid.graph_diagonal_trap",
                  "graph recovers the full lattice under strong 45deg oblique shear")
{
  const int COLS=9, ROWS=7; const int W=560, H=480; const float sq=40.f;
  const float Mcx=W/2.f, Mcy=H/2.f, Icx=W/2.f, Icy=H/2.f;
  const float phi=45.f*(float)M_PI/180.f;
  // image cell axis vectors (equal length L, angle phi between them) and inverse
  const float L=40.f;
  const float ax=L, ay=0.f, bx=L*std::cos(phi), by=L*std::sin(phi);
  const float det = ax*by - ay*bx;                       // |[a b]|
  auto warp = [&](float qx,float qy)->Point32f{          // image px -> model coord
    const float ex=qx-Icx, ey=qy-Icy;
    const float u = ( by*ex - bx*ey)/det;                // M^{-1} (image-center)
    const float v = (-ay*ex + ax*ey)/det;
    return Point32f(Mcx + sq*u, Mcy + sq*v);
  };
  Img8u img = renderBoard(W, H, COLS, ROWS, sq, warp);
  const auto seeds = CheckerboardSaddleDetector().detect(img);

  const CheckerboardGrid gg = recoverCheckerboardGridGraph(seeds, img);
  std::cout << "[checkergrid] graph_trap: " << gg.cols << "x" << gg.rows
            << " count=" << gg.count << " (seeds=" << seeds.size() << ")" << std::endl;
  ICL_TEST_TRUE(fullGrid(gg, COLS-1, ROWS-1));
  ICL_TEST_TRUE(latticeConsistent(gg));
}

// Sub-pixel corner polish: the ChESS parabolic peak lands ~0.3-0.6px off the true
// inner-corner grid under perspective; refineCheckerboardCornersSubPix() pulls it
// to <0.05px. Under a planar perspective view the (col,row) lattice maps to the
// image by an exact homography, so the RMS residual of the best-fit homography IS
// the corner-localisation error — measured before vs after the polish.
ICL_REGISTER_TEST("cv.checkergrid.subpixel_refine_improves_corners",
                  "gradient sub-pixel polish drives corners onto the true homography")
{
  const int COLS=9, ROWS=7; const int W=480, H=400;
  // genuine projective (keystone) warp: output pixel -> source coord. Under a true
  // homography the lattice (col,row) -> image map is itself an exact homography, so
  // its residual isolates corner-localisation error (a non-projective warp would
  // leave a large model-mismatch floor that swamps the sub-pixel gain).
  const Point32f srcC[4]={{0,0},{(float)W,0},{(float)W,(float)H},{0,(float)H}};
  const float ks=0.22f;                                  // top-edge keystone
  const Point32f dstC[4]={{ks*W,0},{(1-ks)*W,0},{(float)W,(float)H},{0,(float)H}};
  const math::Homography2D Hv2s(srcC, dstC, 4);          // apply(outputPx) -> sourcePx
  auto warp = [&](float qx,float qy){ return Hv2s.apply(Point32f(qx,qy)); };
  Img8u img = renderBoard(W, H, COLS, ROWS, 40, warp);

  const auto seeds = CheckerboardSaddleDetector().detect(img);
  CheckerboardGrid g = recoverCheckerboardGrid(seeds, &img);
  ICL_TEST_TRUE(dimsMatch(g, COLS, ROWS));
  ICL_TEST_TRUE(g.complete());

  // RMS residual of the homography (col,row) -> image over all filled cells
  auto homographyResidual = [&](const CheckerboardGrid &gr){
    std::vector<Point32f> latt, im;
    for (int r=0;r<gr.rows;++r) for (int c=0;c<gr.cols;++c)
      if (gr.has(c,r)) { latt.push_back(Point32f((float)c,(float)r)); im.push_back(gr.at(c,r)); }
    const math::Homography2D Hom(im.data(), latt.data(), (int)latt.size());   // apply(latt) -> image
    double e=0; for (size_t i=0;i<latt.size();++i) e += std::pow(Hom.apply(latt[i]).distanceTo(im[i]), 2.0);
    return std::sqrt(e/latt.size());
  };

  const double before = homographyResidual(g);
  refineCheckerboardCornersSubPix(g, img);
  const double after = homographyResidual(g);

  std::cout << "[checkergrid] subpixel: homography residual " << before
            << " -> " << after << " px" << std::endl;
  ICL_TEST_TRUE(after < before*0.4);  // a large (here ~7x) improvement
  ICL_TEST_TRUE(after < 0.15);        // sub-0.15px (the 4x-supersampled render is the
                                      // floor here; the Phase-B harness reaches ~0.01)
}
