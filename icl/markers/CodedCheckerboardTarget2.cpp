// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/markers/CodedCheckerboardTarget2.h>
#include <icl/markers/BCHCode.h>
#include <icl/markers/FiducialDetector.h>
#include <icl/markers/FiducialDetectorPlugin.h>
#include <icl/markers/Fiducial.h>
#include <icl/cv/CheckerboardSaddleDetector.h>
#include <icl/cv/CheckerboardGrid.h>
#include <icl/math/transform/Homography2D.h>
#include <algorithm>
#include <cmath>
#include <map>
#include <memory>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::cv3d;

namespace icl::markers {

  namespace {
    std::string detectorTypeFor2(SquareBCHPreset p) {
      switch (p) {
        case SquareBCHPreset::BCH_4x4_t2_RS: return "bch4x4";
        case SquareBCHPreset::BCH_5x5_t4_RS: return "bch5x5";
        case SquareBCHPreset::BCH_6x6_t4_RS: return "bch6x6";
        case SquareBCHPreset::BCH_3x3_t1:    return "bch3x3";
        default:
          throw ICLException("CodedCheckerboardTarget2: preset has no n×n detector "
                             "plugin (use BCH_4x4_t2_RS, BCH_5x5_t4_RS, BCH_6x6_t4_RS, "
                             "or BCH_3x3_t1)");
      }
    }
    constexpr int MARKER_BORDER = 2;   // must match the plugin's "border width" default
    // markers with a corner within this many px of the image edge are clipped →
    // their decode / corner localisation is unreliable → drop them (false-positive guard)
    constexpr float BORDER_MARGIN_PX = 2.f;
  }

  struct CodedCheckerboardTarget2::Data {
    int cols, rows;
    float squareMM;
    float fill;
    float stubFrac;
    SquareBCHPreset preset;
    SquareBCHCode code;
    std::string detType;

    /// a marker-bearing cell (checker-cell coords) + its parity (0 = white, 1 = black)
    struct Coded { int id, cx, cy, parity; };
    std::vector<Coded> coded;
    std::map<int, std::pair<int,int>> id2cell;   ///< marker id → (cx,cy), across BOTH parities

    std::unique_ptr<FiducialDetector> fd;
    cv::CheckerboardSaddleDetector saddle;

    Data(int c, int r, float sq, float f, SquareBCHPreset p, float stub)
      : cols(c), rows(r), squareMM(sq), fill(f), stubFrac(stub), preset(p),
        code(SquareBCHCode::presetInfo(p).gridSize, SquareBCHCode::presetInfo(p).correctable),
        detType(detectorTypeFor2(p)) {
      // EVERY cell of the whole board (incl. the outer ring) carries a marker —
      // white cells (cx+cy even) a normal marker, black cells (odd) an inverted one.
      // One globally-unique orientation-safe id per cell across both parities.
      std::vector<std::pair<int,int>> cells;
      for (int cy = 0; cy < rows; ++cy)
        for (int cx = 0; cx < cols; ++cx)
          cells.push_back({cx, cy});

      const std::vector<int> ids = code.orientationSafeIds((int)cells.size());
      if (ids.size() < cells.size())
        throw ICLException("CodedCheckerboardTarget2: board needs " + str(cells.size()) +
                           " markers but preset " + SquareBCHCode::presetInfo(p).name +
                           " offers only " + str(ids.size()) + " orientation-safe ids");
      for (size_t k = 0; k < cells.size(); ++k) {
        const int parity = (cells[k].first + cells[k].second) & 1;
        coded.push_back({ids[k], cells[k].first, cells[k].second, parity});
        id2cell[ids[k]] = cells[k];
      }
    }

    FiducialDetector *ensureDetector() {
      if (!fd) {
        std::vector<int> ids; ids.reserve(coded.size());
        for (const auto &c : coded) ids.push_back(c.id);
        fd.reset(new FiducialDetector(detType, ids, ParamMap{{"size", Size(100,100)}}));
        fd->getPlugin()->setPropertyValue("max bch errors", code.correctable());
      }
      return fd.get();
    }
  };

  CodedCheckerboardTarget2::CodedCheckerboardTarget2(int cols, int rows, float squareSizeMM,
                                                     float markerFill, SquareBCHPreset preset,
                                                     float stubDepthFrac)
    : m_data(new Data(cols, rows, squareSizeMM, markerFill, preset, stubDepthFrac)) {}

  CodedCheckerboardTarget2::~CodedCheckerboardTarget2() { delete m_data; }

  FiducialDetector *CodedCheckerboardTarget2::markerDetector() const { return m_data->ensureDetector(); }

  int   CodedCheckerboardTarget2::getCols()       const { return m_data->cols; }
  int   CodedCheckerboardTarget2::getRows()       const { return m_data->rows; }
  float CodedCheckerboardTarget2::getSquareSize() const { return m_data->squareMM; }
  int   CodedCheckerboardTarget2::numMarkers()    const { return (int)m_data->coded.size(); }

  std::vector<Vec> CodedCheckerboardTarget2::modelPoints() const {
    // extended lattice: (cols+1)×(rows+1) grid intersections at ((c-1)·sq,(r-1)·sq) —
    // the interior inner corners PLUS the edge ring made detectable by the stubs.
    const int C = m_data->cols, R = m_data->rows;
    const float sq = m_data->squareMM;
    std::vector<Vec> pts;
    pts.reserve((size_t)(C+1)*(R+1));
    for (int r = 0; r <= R; ++r)
      for (int c = 0; c <= C; ++c)
        pts.push_back(Vec((c-1)*sq, (r-1)*sq, 0.f, 1.f));
    return pts;
  }

  core::Img8u CodedCheckerboardTarget2::generate(const utils::Size &pixelSize) const {
    const int C = m_data->cols, R = m_data->rows;
    const float sd = m_data->stubFrac;
    Img8u img(pixelSize, 1);
    Channel8u d = img[0];

    // checker squares + a 1-square white quiet zone, centred (same layout convention
    // as CodedCheckerboardTarget/CheckerboardTarget so board-mm↔pixel is shared).
    const float px = std::min(pixelSize.width  / float(C + 2),
                              pixelSize.height / float(R + 2));
    const float bw = px*C, bh = px*R;
    const float ox = (pixelSize.width - bw)/2.f, oy = (pixelSize.height - bh)/2.f;
    // continued-checker colour (black if the cell index sum is odd)
    auto checker = [](int ci, int cj) -> float { return ((ci + cj) & 1) ? 0.f : 255.f; };
    auto val = [&](float fx, float fy) -> float {
      const float lx = fx - ox, ly = fy - oy;
      const bool inX = (lx >= 0 && lx < bw), inY = (ly >= 0 && ly < bh);
      if (inX && inY) return checker((int)std::floor(lx/px), (int)std::floor(ly/px));
      if (!inX && !inY) return 255.f;                    // corner quiet zone stays white
      // outside in exactly one axis → an EDGE STUB iff it is within the shallow stub
      // band AND the continued checker would be black there (white continuations are
      // free — the quiet zone is already white).
      if (!inX) {
        const int ci = (lx < 0) ? -1 : C;
        const int cj = std::min(R-1, std::max(0, (int)std::floor(ly/px)));
        const float dout = (lx < 0) ? -lx : (lx - bw);
        if (dout <= sd*px && checker(ci, cj) == 0.f) return 0.f;
        return 255.f;
      }
      const int cj = (ly < 0) ? -1 : R;
      const int ci = std::min(C-1, std::max(0, (int)std::floor(lx/px)));
      const float dout = (ly < 0) ? -ly : (ly - bh);
      if (dout <= sd*px && checker(ci, cj) == 0.f) return 0.f;
      return 255.f;
    };
    // 4x4 supersample → anti-aliased edges so corners localise to sub-pixel
    for (int y = 0; y < pixelSize.height; ++y)
      for (int x = 0; x < pixelSize.width; ++x) {
        float acc = 0; const int SS = 4;
        for (int sy = 0; sy < SS; ++sy) for (int sx = 0; sx < SS; ++sx)
          acc += val(x + (sx+0.5f)/SS - 0.5f, y + (sy+0.5f)/SS - 0.5f);
        d(x, y) = (icl8u)(acc/(SS*SS) + 0.5f);
      }

    // stamp a SHRUNK marker into every cell: white cells host a normal black-on-white
    // marker; black cells host the INVERTED (white-on-black) marker so the cell's black
    // frame margin survives and the boundary saddles are preserved.
    const int mpx = std::max(1, (int)std::lround(m_data->fill * px));
    for (const auto &cc : m_data->coded) {
      Img8u marker = m_data->code.markerImage(cc.id, MARKER_BORDER, Size(mpx, mpx));
      if (cc.parity == 1) {                              // black cell → invert the marker
        Channel8u mc = marker[0];
        for (int i = 0, n = marker.getDim(); i < n; ++i) mc[i] = (icl8u)(255 - mc[i]);
      }
      const Channel8u md = marker[0];
      const int mw = marker.getWidth(), mh = marker.getHeight();
      const float ccx = ox + (cc.cx + 0.5f)*px, ccy = oy + (cc.cy + 0.5f)*px;
      const int x0 = (int)std::lround(ccx - mw/2.f), y0 = (int)std::lround(ccy - mh/2.f);
      for (int j = 0; j < mh; ++j) {
        const int yy = y0 + j; if (yy < 0 || yy >= pixelSize.height) continue;
        for (int i = 0; i < mw; ++i) {
          const int xx = x0 + i; if (xx < 0 || xx >= pixelSize.width) continue;
          d(xx, yy) = md(i, j);
        }
      }
    }
    return img;
  }

  std::vector<CalibrationCorrespondence>
  CodedCheckerboardTarget2::detect(const core::Img8u &image) const {
    Data &D = *m_data;
    const float sq = D.squareMM;
    const float fill = D.fill;                           // uniform: every marker is shrunk

    // saddle seeds (polarity-agnostic; run once on the original frame). refinement and
    // corner geometry all use the original image — only marker IDENTITY uses both polarities.
    const std::vector<cv::CornerSeed> seeds = D.saddle.detect(image);
    if (seeds.empty()) return {};

    // marker keypoints are copied out (not held by pointer) because the two detector
    // passes reuse the detector's internal Fiducial storage — pass 2 invalidates pass 1.
    struct Marker { int cx, cy; Point32f center; Point32f mp[4], ip[4]; };
    std::vector<Marker> ms;

    auto collectPass = [&](const Img8u &img) {
      const std::vector<Fiducial> &fids = D.ensureDetector()->detect(&img);
      if (fids.empty()) return;
      const int IW = img.getWidth(), IH = img.getHeight();
      // solid/garbage decodes duplicate an id → keep only unique-id markers (per pass;
      // cross-pass survivors are dropped geometrically by the RANSAC pose below).
      std::map<int,int> idCount;
      for (const Fiducial &f : fids)
        if (D.id2cell.count(f.getID())) ++idCount[f.getID()];
      for (const Fiducial &f : fids) {
        auto itc = D.id2cell.find(f.getID());
        if (itc == D.id2cell.end() || idCount[f.getID()] != 1) continue;
        const std::vector<Fiducial::KeyPoint> &kps = f.getKeyPoints2D();
        if (kps.size() != 4) continue;
        // drop markers clipped by the frame edge — their decode & corner localisation
        // are unreliable and are a common false-positive source at steep/close poses
        bool clipped = false;
        for (int k = 0; k < 4; ++k) {
          const Point32f &ip = kps[k].imagePos;
          if (ip.x < BORDER_MARGIN_PX || ip.y < BORDER_MARGIN_PX ||
              ip.x > IW-1-BORDER_MARGIN_PX || ip.y > IH-1-BORDER_MARGIN_PX) { clipped = true; break; }
        }
        if (clipped) continue;
        Marker m; m.cx = itc->second.first; m.cy = itc->second.second;
        Point32f c(0,0);
        for (int k = 0; k < 4; ++k) {
          m.mp[k] = kps[k].markerPos; m.ip[k] = kps[k].imagePos;
          c.x += kps[k].imagePos.x; c.y += kps[k].imagePos.y;
        }
        m.center = Point32f(c.x*0.25f, c.y*0.25f);
        ms.push_back(m);
      }
    };

    collectPass(image);                                  // white-cell markers
    { Img8u inv = image; inv.detach();                   // black-cell markers on the inverse
      for (int ch = 0; ch < inv.getChannels(); ++ch) {
        icl8u *p = inv.begin(ch);
        for (int i = 0, n = inv.getDim(); i < n; ++i) p[i] = (icl8u)(255 - p[i]);
      }
      collectPass(inv); }
    if (ms.size() < 4) return {};

    auto fitLocal = [](const Marker &m) { return math::Homography2D::fit(m.mp, m.ip, 4); };

    // (2) robust (RANSAC) image→board pose over marker centres — drops any cross-pass /
    // mis-decoded straggler whose centre disagrees with the consensus.
    std::vector<Point32f> I, B;
    for (const auto &m : ms) { I.push_back(m.center); B.push_back(Point32f((m.cx-0.5f)*sq, (m.cy-0.5f)*sq)); }
    const auto rf = math::Homography2D::robust(I.data(), B.data(), (int)I.size(), 0.35f*sq);
    if (!rf.ok) return {};
    { std::vector<Marker> keep; keep.reserve(rf.inliers.size());
      for (int i : rf.inliers) keep.push_back(ms[i]); ms.swap(keep); }
    math::Homography2D Hib = rf.H;

    // (3) discrete orientation R (per marker-local quadrant → board-corner offset), voted
    // globally then applied locally — distortion-robust, exactly as CodedCheckerboardTarget.
    std::map<int, std::map<std::pair<int,int>,int>> vote;
    for (const auto &m : ms) {
      const math::Homography2D H = fitLocal(m);
      for (int k = 0; k < 4; ++k) {
        const Point32f b = Hib.apply(H.apply(Point32f(m.mp[k].x/fill, m.mp[k].y/fill)));
        const int dcol = (int)std::lround(b.x/sq) - m.cx, drow = (int)std::lround(b.y/sq) - m.cy;
        if (dcol < -1 || dcol > 0 || drow < -1 || drow > 0) continue;
        const int q = (m.mp[k].x > 0 ? 2 : 0) | (m.mp[k].y > 0 ? 1 : 0);
        ++vote[q][{dcol, drow}];
      }
    }
    std::pair<int,int> R[4] = {{0,0},{0,0},{0,0},{0,0}};
    for (int q = 0; q < 4; ++q) {
      int bestc = -1;
      for (const auto &kv : vote[q]) if (kv.second > bestc) { bestc = kv.second; R[q] = kv.first; }
    }

    // (4) label (+1-shifted into the extended lattice) + sub-pixel saddle snap.
    // idx' = cell + R + 1 ∈ [0,C]×[0,R]; the edge ring (idx'=0 or C/R) that plain coded
    // would clip is now kept, backed by the stub-created saddles.
    const int GC = D.cols+1, GR = D.rows+1;
    std::map<std::pair<int,int>, std::pair<Point32f,int>> acc;
    for (const auto &m : ms) {
      const math::Homography2D H = fitLocal(m);
      Point32f pred[4]; std::pair<int,int> idx[4];
      for (int k = 0; k < 4; ++k) {
        pred[k] = H.apply(Point32f(m.mp[k].x/fill, m.mp[k].y/fill));
        const int q = (m.mp[k].x > 0 ? 2 : 0) | (m.mp[k].y > 0 ? 1 : 0);
        idx[k] = { m.cx + R[q].first + 1, m.cy + R[q].second + 1 };
      }
      float tol = 1e18f;
      for (int a = 0; a < 4; ++a)
        for (int b = a+1; b < 4; ++b)
          tol = std::min(tol, std::hypot(pred[a].x-pred[b].x, pred[a].y-pred[b].y));
      tol *= 0.5f;
      for (int k = 0; k < 4; ++k) {
        float best = tol; int bi = -1;
        for (size_t s = 0; s < seeds.size(); ++s) {
          const float dd = std::hypot(pred[k].x-seeds[s].pos.x, pred[k].y-seeds[s].pos.y);
          if (dd < best) { best = dd; bi = (int)s; }
        }
        if (bi < 0) continue;
        auto &e = acc[idx[k]];
        e.first.x += seeds[bi].pos.x; e.first.y += seeds[bi].pos.y; ++e.second;
      }
    }

    // assemble the labelled corners into the (partial) extended lattice and run the same
    // gradient sub-pixel polish CheckerboardTarget uses.
    cv::CheckerboardGrid grid;
    grid.cols = GC; grid.rows = GR;
    grid.points.assign((size_t)GC*GR, Point32f(0,0));
    grid.filled.assign((size_t)GC*GR, 0);
    for (const auto &kv : acc) {
      const int ic = kv.first.first, ir = kv.first.second;
      if (ic < 0 || ir < 0 || ic >= GC || ir >= GR) continue;
      const float inv = 1.f/kv.second.second;
      grid.points[(size_t)ir*GC + ic] = Point32f(kv.second.first.x*inv, kv.second.first.y*inv);
      grid.filled[(size_t)ir*GC + ic] = 1;
      ++grid.count;
    }
    cv::refineCheckerboardCornersSubPix(grid, image);

    // ---- false-positive corner rejection ----
    // A plane under perspective is EXACTLY a homography, so the residual of a robust
    // board→image homography removes perspective entirely and leaves only the SMOOTH lens-
    // distortion field plus ISOLATED mislabel spikes. A centered second difference of that
    // residual field then cancels the smooth distortion too, so a genuine corner's residual
    // equals the midpoint of its opposite neighbours' residuals while a mislabel spikes —
    // perspective- AND distortion-immune, so the threshold can be tight without ever
    // dropping a genuine peripheral corner.
    std::vector<char> kept(grid.filled.begin(), grid.filled.end());
    auto at     = [&](int ic,int ir){ return grid.at(ic, ir); };
    auto keptAt = [&](int ic,int ir){ return ic>=0 && ir>=0 && ic<GC && ir<GR && kept[(size_t)ir*GC+ic]; };
    auto objAt  = [&](int ic,int ir){ return Point32f((ic-1)*sq, (ir-1)*sq); };

    // apparent cell size (median adjacent filled-corner gap) — the rejection scale
    std::vector<float> gaps;
    for (int ir = 0; ir < GR; ++ir)
      for (int ic = 0; ic < GC; ++ic)
        if (kept[(size_t)ir*GC + ic]) {
          const Point32f p = at(ic, ir);
          if (keptAt(ic+1,ir)) { const Point32f q = at(ic+1,ir); gaps.push_back(std::hypot(p.x-q.x, p.y-q.y)); }
          if (keptAt(ic,ir+1)) { const Point32f q = at(ic,ir+1); gaps.push_back(std::hypot(p.x-q.x, p.y-q.y)); }
        }
    float cellPx = 0.f;
    if (!gaps.empty()) { std::nth_element(gaps.begin(), gaps.begin()+gaps.size()/2, gaps.end());
                         cellPx = gaps[gaps.size()/2]; }

    if (cellPx > 0) {
      // robust global homography over the filled corners (fit the bulk; perspective is exact)
      std::vector<Point32f> obj, im;
      for (int ir=0; ir<GR; ++ir) for (int ic=0; ic<GC; ++ic)
        if (kept[(size_t)ir*GC+ic]) { obj.push_back(objAt(ic,ir)); im.push_back(at(ic,ir)); }
      if (obj.size() >= 8) {
        const auto rf = math::Homography2D::robust(obj.data(), im.data(), (int)obj.size(), 0.5f*cellPx);
        if (rf.ok) {
          const math::Homography2D &H = rf.H;
          // residual field r = image − H(object) (perspective removed → smooth + spikes)
          std::vector<Point32f> res((size_t)GC*GR);
          for (int ir=0; ir<GR; ++ir) for (int ic=0; ic<GC; ++ic)
            if (kept[(size_t)ir*GC+ic]) { const Point32f p=at(ic,ir), h=H.apply(objAt(ic,ir));
                                          res[(size_t)ir*GC+ic] = Point32f(p.x-h.x, p.y-h.y); }
          static const int DIR[4][4] = {{-1,0,1,0},{0,-1,0,1},{-1,-1,1,1},{-1,1,1,-1}};
          const float nthr = std::max(3.f, 0.08f*cellPx);  // isolated-spike: tight (distortion-immune)
          const float athr = 0.35f*cellPx;                 // gross magnitude: catches CLUSTERED mislabels
          std::vector<int> drop;                           //   (a shifted patch its neighbours share) —
          for (int ir=0; ir<GR; ++ir) for (int ic=0; ic<GC; ++ic) {   // loose enough to spare distorted periphery
            if (!kept[(size_t)ir*GC+ic]) continue;
            const Point32f r = res[(size_t)ir*GC+ic];
            bool bad = std::hypot(r.x, r.y) > athr;         // gross residual (perspective removed)
            if (!bad) {
              std::vector<float> mx, my;                    // opposite-neighbour residual midpoints
              for (const auto &D : DIR)
                if (keptAt(ic+D[0],ir+D[1]) && keptAt(ic+D[2],ir+D[3])) {
                  const Point32f a = res[(size_t)(ir+D[1])*GC+(ic+D[0])], b = res[(size_t)(ir+D[3])*GC+(ic+D[2])];
                  mx.push_back(0.5f*(a.x+b.x)); my.push_back(0.5f*(a.y+b.y));
                }
              if (mx.size() >= 2) {
                std::nth_element(mx.begin(), mx.begin()+mx.size()/2, mx.end());
                std::nth_element(my.begin(), my.begin()+my.size()/2, my.end());
                bad = std::hypot(r.x-mx[mx.size()/2], r.y-my[my.size()/2]) > nthr;   // isolated sub-cell spike
              }
            }
            if (bad) drop.push_back(ir*GC+ic);
          }
          for (int l : drop) kept[l] = 0;
        }
      }
    }

    std::vector<CalibrationCorrespondence> out;
    out.reserve(grid.count);
    for (int ir=0; ir<GR; ++ir) for (int ic=0; ic<GC; ++ic)
      if (kept[(size_t)ir*GC+ic]) out.push_back({Vec((ic-1)*sq, (ir-1)*sq, 0.f, 1.f), at(ic,ir)});
    return out;
  }

} // namespace icl::markers
