// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/cv/CheckerboardGrid.h>
#include <algorithm>
#include <cmath>
#include <map>
#include <utility>

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

    // Bootstrap the two local axes from the start seed's neighbours. e1 = nearest
    // neighbour (an axis neighbour: the diagonal one is ~1.4x further). e2 = the
    // nearest neighbour whose direction is MOST PERPENDICULAR to e1 — NOT a 90deg
    // rotation of e1: under camera tilt the two board axes are non-orthogonal in
    // the image, and a perpendicular guess would point at a diagonal neighbour
    // and grow the wrong (diagonal) lattice. (The ChESS orientation isn't used at
    // all here — its phase points along the board diagonals.) Both axes are
    // refined from the measured links during growth.
    const Point32f p0 = seeds[start].pos;
    int nb = -1; float nbd2 = 1e30f;
    for (int j = 0; j < n; ++j) if (j != start) {
      const float d2 = dist2(p0, seeds[j].pos);
      if (d2 < nbd2) { nbd2 = d2; nb = j; }
    }
    const Point32f e1_0(seeds[nb].pos.x - p0.x, seeds[nb].pos.y - p0.y);
    const float len1 = std::sqrt(nbd2);
    // e2 = the NEAREST neighbour that is not collinear with e1. The other axis
    // neighbour sits at ~len1; a diagonal is ~1.4x further — so "nearest among the
    // non-collinear" is the second axis, whatever angle it makes with e1 (handles
    // non-orthogonal tilted axes). We only exclude the two ±e1 neighbours.
    int nb2 = -1; float nb2d2 = 1e30f;
    for (int j = 0; j < n; ++j) if (j != start && j != nb) {
      const Point32f v(seeds[j].pos.x - p0.x, seeds[j].pos.y - p0.y);
      const float lv2 = v.x*v.x + v.y*v.y, lv = std::sqrt(lv2);
      if (lv > 1.8f*len1) continue;                                  // near only
      if (std::fabs((v.x*e1_0.x + v.y*e1_0.y)/(lv*len1)) > 0.85f) continue; // skip ±e1
      if (lv2 < nb2d2) { nb2d2 = lv2; nb2 = j; }
    }
    const Point32f e2_0 = (nb2 >= 0)
      ? Point32f(seeds[nb2].pos.x - p0.x, seeds[nb2].pos.y - p0.y)
      : Point32f(-e1_0.y, e1_0.x);   // fallback: perpendicular

    auto findNear = [&](const Point32f &target, float maxr) -> int {
      int best = -1; float bestd2 = maxr*maxr;
      for (int j = 0; j < n; ++j) if (!assigned[j]) {
        const float d2 = dist2(seeds[j].pos, target);
        if (d2 < bestd2) { bestd2 = d2; best = j; }
      }
      return best;
    };

    // Fixed-point growth. cell[(gx,gy)] = seed index. Each pass, every assigned
    // cell estimates its LOCAL step vectors from its own assigned neighbours
    // (falling back to the bootstrap axes) and tries to claim its 4 empty grid
    // neighbours. Re-estimating per cell keeps steps accurate under perspective /
    // distortion (no stale carry), and iterating until nothing new is claimed
    // fills cells a single BFS pass would miss at the tilted far edge.
    std::map<std::pair<int,int>, int> cell;
    assigned[start] = 1; coordX[start] = 0; coordY[start] = 0;
    cell[{0,0}] = start;

    static const int GD[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
    bool changed = true;
    while (changed) {
      changed = false;
      std::vector<int> cur;
      cur.reserve(cell.size());
      for (const auto &kv : cell) cur.push_back(kv.second);
      for (const int s : cur) {
        const int gx = coordX[s], gy = coordY[s];
        const Point32f p = seeds[s].pos;
        // local axes from assigned neighbours, else the bootstrap axes
        Point32f e1 = e1_0, e2 = e2_0;
        if (auto it = cell.find({gx+1,gy}); it != cell.end())
          e1 = Point32f(seeds[it->second].pos.x-p.x, seeds[it->second].pos.y-p.y);
        else if (auto it2 = cell.find({gx-1,gy}); it2 != cell.end())
          e1 = Point32f(p.x-seeds[it2->second].pos.x, p.y-seeds[it2->second].pos.y);
        if (auto it = cell.find({gx,gy+1}); it != cell.end())
          e2 = Point32f(seeds[it->second].pos.x-p.x, seeds[it->second].pos.y-p.y);
        else if (auto it2 = cell.find({gx,gy-1}); it2 != cell.end())
          e2 = Point32f(p.x-seeds[it2->second].pos.x, p.y-seeds[it2->second].pos.y);

        for (const auto &gd : GD) {
          const int dx = gd[0], dy = gd[1], ngx = gx+dx, ngy = gy+dy;
          if (cell.count({ngx,ngy})) continue;                 // already filled
          const Point32f step(dx*e1.x + dy*e2.x, dx*e1.y + dy*e2.y);
          const float steplen = std::sqrt(step.x*step.x + step.y*step.y);
          if (!(steplen > 0)) continue;
          // claim a free seed within 0.6 step of the prediction (a diagonal /
          // 2-step seed sits ~1 step away, so it is not mistaken for a neighbour)
          const int j = findNear(Point32f(p.x+step.x, p.y+step.y), 0.6f*steplen);
          if (j < 0) continue;
          assigned[j] = 1; coordX[j] = ngx; coordY[j] = ngy;
          cell[{ngx,ngy}] = j; changed = true;
        }
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
