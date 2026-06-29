// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/cv/CheckerboardGrid.h>
#include <icl/cv/HungarianAlgorithm.h>
#include <icl/math/transform/Homography2D.h>
#include <icl/utils/Array2D.h>
#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <utility>

using namespace icl::utils;
using namespace icl::core;

namespace icl::cv {
  namespace {
    inline float dist2(const Point32f &a, const Point32f &b) {
      const float dx = a.x-b.x, dy = a.y-b.y; return dx*dx + dy*dy;
    }

    // Lightly-blurred gray of an image + the per-edge confidence used by both the
    // validation pass and guided growth: mean image gradient PERPENDICULAR to an
    // edge, sampled along it, normalised to ~[0,1]. A real checkerboard edge lies
    // on a black/white square border (high); an edge crossing a uniform square
    // (a diagonal, or a gap) scores low.
    struct GrayProbe {
      std::vector<float> g; int W = 0, H = 0;
      explicit GrayProbe(const Img8u &image) {
        W = image.getWidth(); H = image.getHeight(); g.resize((size_t)W*H);
        if (image.getChannels() >= 3) {
          const icl8u *r = image.begin(0), *gr = image.begin(1), *b = image.begin(2);
          for (size_t i = 0; i < g.size(); ++i) g[i] = 0.299f*r[i] + 0.587f*gr[i] + 0.114f*b[i];
        } else {
          const icl8u *d = image.begin(0);
          for (size_t i = 0; i < g.size(); ++i) g[i] = d[i];
        }
      }
      float px(int x, int y) const {
        x = std::min(std::max(x,0), W-1); y = std::min(std::max(y,0), H-1);
        return g[(size_t)y*W + x];
      }
      float blur(float fx, float fy) const {
        const int x = (int)std::floor(fx), y = (int)std::floor(fy);
        float s = 0; for (int dy=-1; dy<=1; ++dy) for (int dx=-1; dx<=1; ++dx) s += px(x+dx, y+dy);
        return s / 9.f;
      }
      // \a perpSpacing is the cell spacing PERPENDICULAR to the edge (the other
      // grid axis). We probe ~0.4 of it to each side, reaching the centres of the
      // black/white cells the edge borders — NOT a fraction of the edge's own
      // length, which under anisotropic foreshortening leaves both probes in the
      // gray border and wrongly scores a real edge low.
      float edgeScore(const Point32f &A, const Point32f &B, float perpSpacing) const {
        const float dx = B.x-A.x, dy = B.y-A.y, L = std::sqrt(dx*dx + dy*dy);
        if (L < 3.f) return 0.f;
        const float nx = -dy/L, ny = dx/L;
        const float d = std::max(2.f, 0.4f*perpSpacing);
        const int M = 7; double acc = 0;
        for (int i = 1; i < M; ++i) {
          const float t = (float)i/M, cx = A.x+dx*t, cy = A.y+dy*t;
          acc += std::fabs(blur(cx+nx*d, cy+ny*d) - blur(cx-nx*d, cy-ny*d));
        }
        return (float)(acc / (M-1) / 255.0);
      }
    };

    // Reject a grown link only when its edge evidence is near zero (clearly
    // crossing a uniform square). Kept low on purpose: the anti-diagonal work is
    // done by the edge-guided AXIS bootstrap; a higher gate here would drop real
    // but weakly-contrasted (e.g. strongly-foreshortened) edges.
    constexpr float LINK_MIN = 0.06f;
  }

  CheckerboardGrid recoverCheckerboardGrid(const std::vector<CornerSeed> &rawSeeds,
                                           const core::Img8u *image) {
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
      const float minSep2 = std::pow(0.35f * s[s.size()/2], 2.f);

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

    std::unique_ptr<GrayProbe> probe;     // guided growth when an image is given
    if (image) probe = std::make_unique<GrayProbe>(*image);

    // grow from the highest-scoring seed
    int start = 0;
    for (int i = 1; i < n; ++i) if (seeds[i].score > seeds[start].score) start = i;
    const Point32f p0 = seeds[start].pos;

    std::vector<int>  coordX(n, 0), coordY(n, 0);
    std::vector<char> assigned(n, 0);

    // Bootstrap the two local axes from the start seed's near neighbours. The
    // pure-geometry heuristic ("nearest" + "nearest non-collinear") fails under
    // strong foreshortening: the board diagonal can be SHORTER than the long axis,
    // so the second axis picks a diagonal and the whole lattice grows diagonally.
    // GUIDED: when an image is given, choose the two axes by EDGE EVIDENCE — a
    // true axis neighbour's connecting edge runs along a black/white border (high
    // score); a diagonal crosses a uniform square (low). Robust to any
    // foreshortening. Without an image, fall back to the geometric heuristic.
    float len1 = 0;
    { float nbd2 = 1e30f; for (int j = 0; j < n; ++j) if (j != start)
        nbd2 = std::min(nbd2, dist2(p0, seeds[j].pos));
      len1 = std::sqrt(nbd2); }

    auto axisKey = [&](int j) -> float {   // higher = better axis candidate
      return probe ? probe->edgeScore(p0, seeds[j].pos, len1)   // len1 ~ a cell spacing
                   : -std::sqrt(dist2(p0, seeds[j].pos));        // nearest
    };
    int a1 = -1; float k1 = -1e30f;
    for (int j = 0; j < n; ++j) if (j != start) {
      const float lv = std::sqrt(dist2(p0, seeds[j].pos));
      if (lv < 0.4f*len1 || lv > 1.8f*len1) continue;          // near (axis/diagonal) only
      const float k = axisKey(j);
      if (k > k1) { k1 = k; a1 = j; }
    }
    if (a1 < 0) return grid;
    const Point32f e1_0(seeds[a1].pos.x - p0.x, seeds[a1].pos.y - p0.y);
    int a2 = -1; float k2 = -1e30f;
    for (int j = 0; j < n; ++j) if (j != start && j != a1) {
      const Point32f v(seeds[j].pos.x - p0.x, seeds[j].pos.y - p0.y);
      const float lv = std::sqrt(v.x*v.x + v.y*v.y);
      if (lv < 0.4f*len1 || lv > 1.8f*len1) continue;
      if (std::fabs((v.x*e1_0.x + v.y*e1_0.y)/(lv*len1)) > 0.85f) continue;  // skip ±e1
      const float k = axisKey(j);
      if (k > k2) { k2 = k; a2 = j; }
    }
    const Point32f e2_0 = (a2 >= 0)
      ? Point32f(seeds[a2].pos.x - p0.x, seeds[a2].pos.y - p0.y)
      : Point32f(-e1_0.y, e1_0.x);   // fallback: perpendicular

    // claim the unassigned seed near `target`: with an image pick the highest
    // edge-score candidate within range (and reject a weak/through-square link);
    // without, pick the nearest.
    auto claim = [&](const Point32f &from, const Point32f &target,
                     float maxr, float perpSpacing) -> int {
      const float r2 = maxr*maxr;
      int best = -1; float bestd2 = r2;
      for (int j = 0; j < n; ++j) if (!assigned[j]) {     // geometric: nearest in range
        const float d2 = dist2(seeds[j].pos, target);
        if (d2 < bestd2) { bestd2 = d2; best = j; }
      }
      // image evidence is used only to REJECT a link that crosses a uniform
      // square (the axis choice is already edge-guided at the bootstrap); it does
      // not override the geometric pick among real candidates.
      if (probe && best >= 0 && probe->edgeScore(from, seeds[best].pos, perpSpacing) < LINK_MIN)
        return -1;
      return best;
    };

    // Fixed-point growth. Each pass, every assigned cell estimates its LOCAL step
    // vectors from its own assigned neighbours (else the bootstrap axes) and tries
    // to claim its 4 empty grid neighbours, iterating until nothing new appears.
    std::map<std::pair<int,int>, int> cell;
    assigned[start] = 1; coordX[start] = 0; coordY[start] = 0;
    cell[{0,0}] = start;

    static const int GD[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
    bool changed = true;
    while (changed) {
      changed = false;
      std::vector<int> cur; cur.reserve(cell.size());
      for (const auto &kv : cell) cur.push_back(kv.second);
      for (const int s : cur) {
        const int gx = coordX[s], gy = coordY[s];
        const Point32f p = seeds[s].pos;
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
          if (cell.count({ngx,ngy})) continue;
          const Point32f step(dx*e1.x + dy*e2.x, dx*e1.y + dy*e2.y);
          const float steplen = std::sqrt(step.x*step.x + step.y*step.y);
          if (!(steplen > 0)) continue;
          // perpendicular spacing = the OTHER axis's local length
          const Point32f &perp = dx ? e2 : e1;
          const float perpLen = std::sqrt(perp.x*perp.x + perp.y*perp.y);
          const int j = claim(p, Point32f(p.x+step.x, p.y+step.y), 0.6f*steplen, perpLen);
          if (j < 0) continue;
          assigned[j] = 1; coordX[j] = ngx; coordY[j] = ngy;
          cell[{ngx,ngy}] = j; changed = true;
        }
      }
    }

    // normalise labels to [0,cols)x[0,rows) and fill the grid (keep the
    // higher-scoring seed when two claim the same cell)
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
    for (char f : grid.filled) grid.count += f;
    return grid;
  }

  CheckerboardGrid refineCheckerboardGrid(const CheckerboardGrid &in,
                                          const std::vector<CornerSeed> &seeds,
                                          const core::Img8u *image) {
    // need a 2D lattice to constrain a homography
    if (in.count < 4 || in.cols < 2 || in.rows < 2) return in;

    // (col,row) -> image correspondences from the filled cells
    std::vector<Point32f> latt, img;
    latt.reserve(in.count); img.reserve(in.count);
    for (int r = 0; r < in.rows; ++r)
      for (int c = 0; c < in.cols; ++c)
        if (in.has(c, r)) { latt.push_back(Point32f((float)c,(float)r)); img.push_back(in.at(c,r)); }
    if ((int)latt.size() < 4) return in;

    using math::Homography2D;
    // Homography2D(x,y) maps y->x (apply(y)~x); we want apply(src)~dst, so the
    // (col,row) lattice is `src` and the image positions are `dst`.
    auto fit = [](const std::vector<Point32f> &src, const std::vector<Point32f> &dst) {
      return Homography2D(dst.data(), src.data(), (int)src.size());
    };

    // global cell spacing (median of filled right/down neighbour steps)
    float spacing = 0.f;
    { std::vector<float> sp;
      auto len = [](const Point32f &a, const Point32f &b){
        return std::sqrt((a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y)); };
      for (int r=0;r<in.rows;++r) for (int c=0;c<in.cols;++c) if (in.has(c,r)) {
        if (in.has(c+1,r)) sp.push_back(len(in.at(c,r), in.at(c+1,r)));
        if (in.has(c,r+1)) sp.push_back(len(in.at(c,r), in.at(c,r+1)));
      }
      if (sp.empty()) return in;
      std::nth_element(sp.begin(), sp.begin()+sp.size()/2, sp.end());
      spacing = sp[sp.size()/2];
    }
    if (!(spacing > 1.f)) return in;

    // robust homography: refit a couple of times, dropping high-residual cells
    Homography2D H = fit(latt, img);
    for (int iter = 0; iter < 2; ++iter) {
      std::vector<float> res(latt.size());
      std::vector<float> srt;
      for (size_t i=0;i<latt.size();++i) {
        const Point32f p = H.apply(latt[i]);
        res[i] = std::sqrt((p.x-img[i].x)*(p.x-img[i].x)+(p.y-img[i].y)*(p.y-img[i].y));
        srt.push_back(res[i]);
      }
      std::nth_element(srt.begin(), srt.begin()+srt.size()/2, srt.end());
      const float thr = std::max(0.25f*spacing, 3.f*srt[srt.size()/2]);
      std::vector<Point32f> a, b;
      for (size_t i=0;i<latt.size();++i) if (res[i] <= thr) { a.push_back(latt[i]); b.push_back(img[i]); }
      if ((int)a.size() < 4 || a.size() == latt.size()) break;
      H = fit(a, b);
    }

    // predicted node positions for the whole current extent
    const int C = in.cols, R = in.rows, J = C*R;
    std::vector<Point32f> node(J);
    for (int r=0;r<R;++r) for (int c=0;c<C;++c)
      node[(size_t)r*C+c] = H.apply(Point32f((float)c,(float)r));

    // prune seeds to those near some node (keeps the assignment matrix small)
    const float keepR2 = std::pow(0.7f*spacing, 2.f);
    std::vector<Point32f> sp;
    for (const auto &s : seeds) {
      float best = 1e30f;
      for (const auto &nd : node) { const float dx=s.pos.x-nd.x, dy=s.pos.y-nd.y;
        best = std::min(best, dx*dx+dy*dy); }
      if (best < keepR2) sp.push_back(s.pos);
    }
    const int I = (int)sp.size();
    // too few seeds, or a matrix too large to be reasonable → leave grid as-is
    if (I < 4 || I + J > 600) return in;

    // Hungarian assignment with rejection. Square cost matrix of size D=I+J:
    //   x-axis (width):  [0,I) seeds, then [I,I+J) per-node "unmatched" dummies
    //   y-axis (height): [0,J) nodes, then [J,J+I) per-seed "unmatched" dummies
    // A real seed->node cost is its distance if within the gate, else LARGE; a
    // dummy lets a seed (or node) stay unmatched at cost DROP. Hungarian then
    // snaps each node to its nearest in-gate seed and drops the rest.
    using utils::Array2D;
    const int D = I + J;
    const double gate = 0.4*spacing, DROP = 0.5*spacing, LARGE = 1e6*(spacing+1.0);
    Array2D<icl64f> cost(D, D);
    for (int x=0;x<D;++x) for (int y=0;y<D;++y) cost(x,y) = LARGE;
    for (int i=0;i<I;++i)
      for (int j=0;j<J;++j) {
        const double dx=sp[i].x-node[j].x, dy=sp[i].y-node[j].y, d=std::sqrt(dx*dx+dy*dy);
        cost(i,j) = (d <= gate) ? d : LARGE;
      }
    for (int i=0;i<I;++i) cost(i, J+i) = DROP;          // seed i unmatched
    for (int j=0;j<J;++j) cost(I+j, j) = DROP;          // node j unmatched
    for (int x=I;x<D;++x) for (int y=J;y<D;++y) cost(x,y) = 0.0;  // dummy-dummy

    const std::vector<int> asg = cv::HungarianAlgorithm<icl64f>::apply(cost, true);

    // rebuild lattice points from the assignment (seed x -> node y if y<J)
    std::vector<Point32f> pts((size_t)J, Point32f(0,0));
    std::vector<char>     fil((size_t)J, 0);
    for (int i=0;i<I && i<(int)asg.size();++i) {
      const int y = asg[i];
      if (y >= 0 && y < J && cost(i,y) < LARGE) { pts[y] = sp[i]; fil[y] = 1; }
    }

    // trim weakly-supported / contrast-free outer rows & columns
    std::unique_ptr<GrayProbe> probe;
    if (image) probe = std::make_unique<GrayProbe>(*image);
    int cols = C, rows = R;
    auto idx = [&](int c, int r){ return (size_t)r*cols + c; };
    constexpr float SUPPORT_MIN = 0.5f, EDGE_MIN = 0.12f;
    auto rankBad = [&](bool isCol, int k) -> bool {
      const int len = isCol ? rows : cols;
      int nfill = 0; for (int t=0;t<len;++t) if (fil[isCol?idx(k,t):idx(t,k)]) ++nfill;
      if ((float)nfill / len < SUPPORT_MIN) return true;
      if (!probe) return false;
      // mean perpendicular-gradient score of edges joining this rank to its
      // inward neighbour; a phantom border crosses uniform squares → low score
      const int kin = isCol ? (k==0 ? 1 : cols-2) : (k==0 ? 1 : rows-2);
      double acc=0; int m=0;
      for (int t=0;t<len;++t) {
        const size_t a = isCol?idx(k,t):idx(t,k), b = isCol?idx(kin,t):idx(t,kin);
        if (fil[a] && fil[b]) { acc += probe->edgeScore(pts[a], pts[b], spacing); ++m; }
      }
      return m && (acc/m) < EDGE_MIN;
    };
    auto dropCol = [&](int k){
      std::vector<Point32f> np((size_t)(cols-1)*rows); std::vector<char> nf((size_t)(cols-1)*rows,0);
      for (int r=0;r<rows;++r) for (int c=0,nc=0;c<cols;++c) if (c!=k) {
        np[(size_t)r*(cols-1)+nc]=pts[idx(c,r)]; nf[(size_t)r*(cols-1)+nc]=fil[idx(c,r)]; ++nc; }
      pts.swap(np); fil.swap(nf); --cols;
    };
    auto dropRow = [&](int k){
      std::vector<Point32f> np((size_t)cols*(rows-1)); std::vector<char> nf((size_t)cols*(rows-1),0);
      for (int r=0,nr=0;r<rows;++r) if (r!=k) { for (int c=0;c<cols;++c) {
        np[(size_t)nr*cols+c]=pts[idx(c,r)]; nf[(size_t)nr*cols+c]=fil[idx(c,r)]; } ++nr; }
      pts.swap(np); fil.swap(nf); --rows;
    };
    while (cols > 2 && rows > 2) {
      if      (rankBad(true,  0))       dropCol(0);
      else if (rankBad(true,  cols-1))  dropCol(cols-1);
      else if (rankBad(false, 0))       dropRow(0);
      else if (rankBad(false, rows-1))  dropRow(rows-1);
      else break;
    }

    CheckerboardGrid out;
    out.cols = cols; out.rows = rows;
    out.points = std::move(pts); out.filled = std::move(fil);
    out.count = 0; for (char f : out.filled) out.count += f;
    return out.count >= 4 ? out : in;
  }

  void scoreCheckerboardGridEdges(CheckerboardGrid &grid, const core::Img8u &image) {
    const int N = grid.cols*grid.rows;
    grid.edgeRight.assign((size_t)N, -1.f);
    grid.edgeDown .assign((size_t)N, -1.f);
    if (grid.empty()) return;
    const GrayProbe probe(image);
    auto len = [](const Point32f &a, const Point32f &b){
      return std::sqrt((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y));
    };
    // perpendicular (other-axis) spacing at cell (c,r): mean of the available
    // neighbour steps in that axis, else the edge length itself
    auto perpV = [&](int c, int r, float fallback) {   // vertical spacing (for a horizontal edge)
      float s = 0; int k = 0;
      if (grid.has(c,r+1)) { s += len(grid.at(c,r), grid.at(c,r+1)); ++k; }
      if (grid.has(c,r-1)) { s += len(grid.at(c,r), grid.at(c,r-1)); ++k; }
      return k ? s/k : fallback;
    };
    auto perpH = [&](int c, int r, float fallback) {   // horizontal spacing (for a vertical edge)
      float s = 0; int k = 0;
      if (grid.has(c+1,r)) { s += len(grid.at(c,r), grid.at(c+1,r)); ++k; }
      if (grid.has(c-1,r)) { s += len(grid.at(c,r), grid.at(c-1,r)); ++k; }
      return k ? s/k : fallback;
    };
    for (int r = 0; r < grid.rows; ++r)
      for (int c = 0; c < grid.cols; ++c) {
        if (!grid.has(c,r)) continue;
        const size_t idx = (size_t)r*grid.cols + c;
        if (grid.has(c+1,r)) {
          const float L = len(grid.at(c,r), grid.at(c+1,r));
          grid.edgeRight[idx] = probe.edgeScore(grid.at(c,r), grid.at(c+1,r), perpV(c,r,L));
        }
        if (grid.has(c,r+1)) {
          const float L = len(grid.at(c,r), grid.at(c,r+1));
          grid.edgeDown[idx]  = probe.edgeScore(grid.at(c,r), grid.at(c,r+1), perpH(c,r,L));
        }
      }
  }

} // namespace icl::cv
