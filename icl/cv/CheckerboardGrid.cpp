// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/cv/CheckerboardGrid.h>
#include <algorithm>
#include <cmath>
#include <queue>

using namespace icl::utils;

namespace icl::cv {
  namespace {
    inline float dist2(const Point32f &a, const Point32f &b) {
      const float dx = a.x-b.x, dy = a.y-b.y; return dx*dx + dy*dy;
    }
  }

  CheckerboardGrid recoverCheckerboardGrid(const std::vector<CornerSeed> &rawSeeds) {
    CheckerboardGrid grid;
    if ((int)rawSeeds.size() < 4) return grid;

    // Dedup near-duplicate detections: a single corner can yield two peaks that
    // both survive NMS (when farther apart than nmsRadius) yet sit well within
    // one cell — they would later collide in the lattice and pull a cell off its
    // true position. Drop any seed within 0.35x the median spacing of an
    // already-kept, higher-scoring seed.
    std::vector<CornerSeed> seeds;
    {
      const int m = (int)rawSeeds.size();
      std::vector<float> nn(m, 1e30f);
      for (int i = 0; i < m; ++i)
        for (int j = 0; j < m; ++j) if (j != i)
          nn[i] = std::min(nn[i], dist2(rawSeeds[i].pos, rawSeeds[j].pos));
      std::vector<float> s; s.reserve(m);
      for (float d2 : nn) s.push_back(std::sqrt(d2));
      std::nth_element(s.begin(), s.begin()+s.size()/2, s.end());
      const float minSep = 0.35f * s[s.size()/2];
      const float minSep2 = minSep*minSep;

      std::vector<int> order(m);
      for (int i = 0; i < m; ++i) order[i] = i;
      std::sort(order.begin(), order.end(),
                [&](int a, int b){ return rawSeeds[a].score > rawSeeds[b].score; });
      for (int oi : order) {
        bool dup = false;
        for (const auto &k : seeds)
          if (dist2(k.pos, rawSeeds[oi].pos) < minSep2) { dup = true; break; }
        if (!dup) seeds.push_back(rawSeeds[oi]);
      }
    }
    const int n = (int)seeds.size();
    if (n < 4) return grid;

    // Grow the lattice from the highest-scoring seed. Each queued node carries
    // its local image-space step vectors e1,e2 for grid +x,+y, refined from the
    // actually measured links so the walk tracks perspective + lens distortion.
    int start = 0;
    for (int i = 1; i < n; ++i) if (seeds[i].score > seeds[start].score) start = i;

    std::vector<int>  coordX(n, 0), coordY(n, 0);
    std::vector<char> assigned(n, 0);

    // initial axes from the start seed's NEAREST neighbour: on a checkerboard the
    // nearest corner is an axis neighbour (the diagonal one is ~1.4x further), so
    // this direction IS a grid axis. (The ChESS `orientation` is NOT used here:
    // its 2nd-harmonic phase points along the board *diagonals*, 45deg off the
    // axes, which would grow only the same-colour diagonal sub-lattice.) The
    // perpendicular seeds e2; both are refined from measured links during growth.
    int nb = start; float nbd2 = 1e30f;
    for (int j = 0; j < n; ++j) if (j != start) {
      const float d2 = dist2(seeds[start].pos, seeds[j].pos);
      if (d2 < nbd2) { nbd2 = d2; nb = j; }
    }
    const Point32f e1_0(seeds[nb].pos.x - seeds[start].pos.x,
                        seeds[nb].pos.y - seeds[start].pos.y);
    const Point32f e2_0(-e1_0.y, e1_0.x);   // perpendicular, same length

    struct Node { int idx, gx, gy; Point32f e1, e2; };
    std::queue<Node> q;
    assigned[start] = 1;
    q.push({start, 0, 0, e1_0, e2_0});

    auto findNear = [&](const Point32f &target, float maxr) -> int {
      int best = -1; float bestd2 = maxr*maxr;
      for (int j = 0; j < n; ++j) if (!assigned[j]) {
        const float d2 = dist2(seeds[j].pos, target);
        if (d2 < bestd2) { bestd2 = d2; best = j; }
      }
      return best;
    };

    static const int GD[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
    while (!q.empty()) {
      const Node cur = q.front(); q.pop();
      const Point32f p = seeds[cur.idx].pos;
      for (const auto &gd : GD) {
        const int dx = gd[0], dy = gd[1];
        const Point32f step(dx*cur.e1.x + dy*cur.e2.x, dx*cur.e1.y + dy*cur.e2.y);
        const float steplen = std::sqrt(step.x*step.x + step.y*step.y);
        if (!(steplen > 0)) continue;
        // accept a free seed within 0.6 step of the predicted neighbour position
        // (excludes diagonal/2-step seeds, which sit ~1 step away)
        const int j = findNear(Point32f(p.x+step.x, p.y+step.y), 0.6f*steplen);
        if (j < 0) continue;
        assigned[j] = 1;
        coordX[j] = cur.gx + dx; coordY[j] = cur.gy + dy;
        // refine: the measured edge replaces the axis we came in on; the
        // perpendicular axis is carried from the parent (refined when used).
        Point32f e1 = cur.e1, e2 = cur.e2;
        const Point32f meas(seeds[j].pos.x - p.x, seeds[j].pos.y - p.y);
        if      (dx ==  1) e1 = meas;
        else if (dx == -1) e1 = Point32f(-meas.x, -meas.y);
        else if (dy ==  1) e2 = meas;
        else if (dy == -1) e2 = Point32f(-meas.x, -meas.y);
        q.push({j, coordX[j], coordY[j], e1, e2});
      }
    }

    // 3) normalise labels to [0,cols)x[0,rows) and fill the grid
    int minx = 1<<30, miny = 1<<30, maxx = -(1<<30), maxy = -(1<<30), cnt = 0;
    for (int i = 0; i < n; ++i) if (assigned[i]) {
      minx = std::min(minx, coordX[i]); maxx = std::max(maxx, coordX[i]);
      miny = std::min(miny, coordY[i]); maxy = std::max(maxy, coordY[i]); ++cnt;
    }
    if (cnt < 4) return grid;

    grid.cols = maxx - minx + 1;
    grid.rows = maxy - miny + 1;
    grid.points.assign((size_t)grid.cols*grid.rows, Point32f(0,0));
    grid.filled.assign((size_t)grid.cols*grid.rows, 0);
    // Place each assigned seed; if two seeds claim the same cell (a spurious
    // detection linked next to a real corner), keep the higher-scoring one.
    std::vector<float> cellScore((size_t)grid.cols*grid.rows, -1.f);
    for (int i = 0; i < n; ++i) if (assigned[i]) {
      const size_t idx = (size_t)(coordY[i]-miny)*grid.cols + (coordX[i]-minx);
      if (seeds[i].score > cellScore[idx]) {
        cellScore[idx] = seeds[i].score;
        grid.points[idx] = seeds[i].pos;
        grid.filled[idx] = 1;
      }
    }
    grid.count = 0;
    for (char f : grid.filled) grid.count += f;   // unique filled cells
    return grid;
  }

} // namespace icl::cv
