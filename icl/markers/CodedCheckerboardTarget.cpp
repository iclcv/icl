// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/markers/CodedCheckerboardTarget.h>
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
    // FiducialDetector plugin type string for a preset (only presets with a
    // registered n×n plugin can back a coded checkerboard)
    std::string detectorTypeFor(SquareBCHPreset p) {
      switch (p) {
        case SquareBCHPreset::BCH_4x4_t2_RS: return "bch4x4";
        case SquareBCHPreset::BCH_5x5_t4_RS: return "bch5x5";
        case SquareBCHPreset::BCH_6x6_t4_RS: return "bch6x6";
        case SquareBCHPreset::BCH_3x3_t1:    return "bch3x3";
        default:
          throw ICLException("CodedCheckerboardTarget: preset has no n×n detector "
                             "plugin (use BCH_4x4_t2_RS, BCH_5x5_t4_RS, BCH_6x6_t4_RS, "
                             "or BCH_3x3_t1)");
      }
    }
    // marker border in code cells; must match the plugin's "border width" default
    constexpr int MARKER_BORDER = 2;
  }

  struct CodedCheckerboardTarget::Data {
    int cols, rows;
    float squareMM;
    float fill;
    SquareBCHPreset preset;
    MarkerCells markerCells;     ///< which checker cells host the markers
    SquareBCHCode code;          ///< renders the marker patterns for generate()
    std::string detType;         ///< FiducialDetector plugin type

    /// a marker-bearing interior white cell (checker-cell coords)
    struct Coded { int id, cx, cy; };
    std::vector<Coded> coded;
    std::map<int, std::pair<int,int>> id2cell;   ///< marker id → (cx,cy)

    // detection state (lazy; mutated from the const detect())
    std::unique_ptr<FiducialDetector> fd;
    cv::CheckerboardSaddleDetector saddle;

    Data(int c, int r, float sq, float f, SquareBCHPreset p, MarkerCells mc, bool border)
      : cols(c), rows(r), squareMM(sq), fill(f), preset(p), markerCells(mc),
        code(SquareBCHCode::presetInfo(p).gridSize, SquareBCHCode::presetInfo(p).correctable),
        detType(detectorTypeFor(p)) {
      // Interior cells (cx∈[1,cols-2], cy∈[1,rows-2]) have 4 surrounding inner
      // corners; \a border also codes the OUTER ring (cx∈[0,cols-1], cy∈[0,rows-1]),
      // whose cells surround only 1–2 inner corners — detect() clips the rest, and
      // the edge corners stay labelled when the board overruns the frame.
      // White cells have (cx+cy) even, black cells odd.
      const int parity = (markerCells == MarkerCells::White) ? 0 : 1;
      const int x0 = border ? 0 : 1, x1 = border ? cols-1 : cols-2;
      const int y0 = border ? 0 : 1, y1 = border ? rows-1 : rows-2;
      std::vector<std::pair<int,int>> cells;
      for (int cy = y0; cy <= y1; ++cy)
        for (int cx = x0; cx <= x1; ++cx)
          if (((cx + cy) & 1) == parity) cells.push_back({cx, cy});

      // assign each cell one ORIENTATION-safe id, so every marker's pose (and thus
      // its surrounding corner labels) is unambiguous under rotation.
      const std::vector<int> ids = code.orientationSafeIds((int)cells.size());
      if (ids.size() < cells.size())
        throw ICLException("CodedCheckerboardTarget: board needs " + str(cells.size()) +
                           " markers but preset " + SquareBCHCode::presetInfo(p).name +
                           " offers only " + str(ids.size()) + " orientation-safe ids");
      for (size_t k = 0; k < cells.size(); ++k) {
        coded.push_back({ids[k], cells[k].first, cells[k].second});
        id2cell[ids[k]] = cells[k];
      }
    }

    /// build the marker FiducialDetector on first use (over exactly this board's
    /// ids, at the code's full error-correction).
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

  CodedCheckerboardTarget::CodedCheckerboardTarget(int cols, int rows, float squareSizeMM,
                                                   float markerFill, SquareBCHPreset preset,
                                                   MarkerCells markerCells, bool includeBorderMarkers)
    : m_data(new Data(cols, rows, squareSizeMM, markerFill, preset, markerCells, includeBorderMarkers)) {}

  CodedCheckerboardTarget::~CodedCheckerboardTarget() { delete m_data; }

  FiducialDetector *CodedCheckerboardTarget::markerDetector() const { return m_data->ensureDetector(); }

  int   CodedCheckerboardTarget::getCols()       const { return m_data->cols; }
  int   CodedCheckerboardTarget::getRows()       const { return m_data->rows; }
  float CodedCheckerboardTarget::getSquareSize() const { return m_data->squareMM; }
  int   CodedCheckerboardTarget::numMarkers()    const { return (int)m_data->coded.size(); }

  std::vector<Vec> CodedCheckerboardTarget::modelPoints() const {
    // the checker inner-corner lattice — identical to CheckerboardTarget (markers
    // don't contribute calibration corners)
    const int C = m_data->cols, R = m_data->rows;
    const float sq = m_data->squareMM;
    std::vector<Vec> pts;
    pts.reserve((size_t)(C-1)*(R-1));
    for (int r = 0; r < R-1; ++r)
      for (int c = 0; c < C-1; ++c)
        pts.push_back(Vec(c*sq, r*sq, 0.f, 1.f));
    return pts;
  }

  core::Img8u CodedCheckerboardTarget::generate(const utils::Size &pixelSize) const {
    const int C = m_data->cols, R = m_data->rows;
    Img8u img(pixelSize, 1);
    Channel8u d = img[0];

    // checker squares + a 1-square white quiet zone, centred and isotropic (same
    // layout convention as CheckerboardTarget so board-mm↔pixel is shared below)
    const float px = std::min(pixelSize.width  / float(C + 2),
                              pixelSize.height / float(R + 2));     // px per square
    const float bw = px*C, bh = px*R;
    const float ox = (pixelSize.width - bw)/2.f, oy = (pixelSize.height - bh)/2.f;
    auto val = [&](float fx, float fy) -> float {
      const float lx = fx - ox, ly = fy - oy;
      if (lx < 0 || ly < 0 || lx >= bw || ly >= bh) return 255.f;   // quiet zone
      return ((int(lx/px) + int(ly/px)) & 1) ? 0.f : 255.f;
    };
    // 4x4 supersample → anti-aliased edges so corners localise to sub-pixel
    for (int y = 0; y < pixelSize.height; ++y)
      for (int x = 0; x < pixelSize.width; ++x) {
        float acc = 0; const int SS = 4;
        for (int sy = 0; sy < SS; ++sy) for (int sx = 0; sx < SS; ++sx)
          acc += val(x + (sx+0.5f)/SS - 0.5f, y + (sy+0.5f)/SS - 0.5f);
        d(x, y) = (icl8u)(acc/(SS*SS) + 0.5f);
      }

    // stamp a BCH marker into each coded cell. WHITE marker cells host a shrunk
    // marker (fill<1) so the white square frames the black-bordered marker and
    // separates it from the black neighbours. BLACK marker cells are REPLACED by a
    // full-cell marker: it's just an ordinary black marker whose white neighbour
    // squares provide the contrast — no inversion, detection is unchanged.
    const bool black = (m_data->markerCells == MarkerCells::Black);
    const int mpx = black ? std::max(1, (int)std::lround(px))
                          : std::max(1, (int)std::lround(m_data->fill * px));
    for (const auto &cc : m_data->coded) {
      const Img8u marker = m_data->code.markerImage(cc.id, MARKER_BORDER, Size(mpx, mpx));
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
  CodedCheckerboardTarget::detect(const core::Img8u &image) const {
    Data &D = *m_data;
    const float sq = D.squareMM;
    // fraction of the cell the marker fills: black-cell markers replace the whole
    // cell (their quad corners ARE the checker corners), white-cell markers are
    // shrunk. Corner prediction extrapolates by 1/fill, so it must match generate().
    const float fill = (D.markerCells == MarkerCells::Black) ? 1.f : D.fill;

    // Build the detector lazily (full error-correction — false positives are
    // rejected geometrically below, not by decoder strictness).
    const std::vector<Fiducial> &fids = D.ensureDetector()->detect(&image);
    if (fids.empty()) return {};
    const std::vector<cv::CornerSeed> seeds = D.saddle.detect(image);
    if (seeds.empty()) return {};

    // (1) WELL-IDENTIFIED MARKERS. Every solid black checker square decodes to the
    // SAME fixed id (all-black → decode(whitening)), so false positives are
    // DUPLICATE ids; genuine markers have unique ids. Keep only unique-id markers.
    std::map<int,int> idCount;
    for (const Fiducial &f : fids)
      if (D.id2cell.count(f.getID())) ++idCount[f.getID()];

    struct Marker { const Fiducial *f; int cx, cy; Point32f center; };
    std::vector<Marker> ms;
    for (const Fiducial &f : fids) {
      auto itc = D.id2cell.find(f.getID());
      if (itc == D.id2cell.end() || idCount[f.getID()] != 1) continue;
      const std::vector<Fiducial::KeyPoint> &kps = f.getKeyPoints2D();
      if (kps.size() != 4) continue;
      Point32f c(0,0);
      for (int k = 0; k < 4; ++k) { c.x += kps[k].imagePos.x; c.y += kps[k].imagePos.y; }
      ms.push_back({&f, itc->second.first, itc->second.second, Point32f(c.x*0.25f, c.y*0.25f)});
    }
    if (ms.size() < 4) return {};

    // (2) BOOTSTRAP THE BOARD POSE from those anchors via a ROBUST (RANSAC)
    // image→board homography over the marker centres: it fits the consensus pose
    // AND drops any marker whose centre disagrees (a rare noise-induced mis-decode)
    // in one step, without the outliers pulling the fit. This is what lets the
    // decoder run permissively without false positives leaking through.
    std::vector<Point32f> I, B;
    for (const auto &m : ms) { I.push_back(m.center); B.push_back(Point32f((m.cx-0.5f)*sq, (m.cy-0.5f)*sq)); }
    const auto rf = math::Homography2D::robust(I.data(), B.data(), (int)I.size(), 0.35f*sq);
    if (!rf.ok) return {};                              // no >=4-marker consensus pose
    { std::vector<Marker> keep; keep.reserve(rf.inliers.size());
      for (int i : rf.inliers) keep.push_back(ms[i]); ms.swap(keep); }
    math::Homography2D Hib = rf.H;                      // already refit on the inliers

    // (3) DISCRETE ORIENTATION R. A square-BCH marker's decoded frame is rotated
    // from the board axes by a fixed 90° multiple that DIFFERS BY CODE. Recover it
    // once by voting (per marker-local quadrant, which board inner-corner offset
    // Hib rounds to). R is a discrete, distortion-independent property, so it can
    // then be applied LOCALLY to every marker — including ones at the strongly
    // distorted image corners, where the global homography would mis-round a label.
    std::map<int, std::map<std::pair<int,int>,int>> vote;   // quadrant → offset → count
    auto localH = [](const Fiducial *f, Point32f mp[4]) {
      const std::vector<Fiducial::KeyPoint> &kps = f->getKeyPoints2D();
      Point32f ip[4];
      for (int k = 0; k < 4; ++k) { mp[k] = kps[k].markerPos; ip[k] = kps[k].imagePos; }
      return math::Homography2D::fit(mp, ip, 4);          // apply(marker-mm)=image-px
    };
    for (const auto &m : ms) {
      Point32f mp[4]; const math::Homography2D H = localH(m.f, mp);
      for (int k = 0; k < 4; ++k) {
        const Point32f b = Hib.apply(H.apply(Point32f(mp[k].x/fill, mp[k].y/fill)));
        const int dcol = (int)std::lround(b.x/sq) - m.cx, drow = (int)std::lround(b.y/sq) - m.cy;
        if (dcol < -1 || dcol > 0 || drow < -1 || drow > 0) continue;     // implausible
        const int q = (mp[k].x > 0 ? 2 : 0) | (mp[k].y > 0 ? 1 : 0);
        ++vote[q][{dcol, drow}];
      }
    }
    std::pair<int,int> R[4] = {{0,0},{0,0},{0,0},{0,0}};
    for (int q = 0; q < 4; ++q) {
      int bestc = -1;
      for (const auto &kv : vote[q]) if (kv.second > bestc) { bestc = kv.second; R[q] = kv.first; }
    }

    // (4) LABEL (local: cell + R, distortion-robust) + POSITION (local homography
    // + sub-pixel saddle snap). A corner is shared by up to 4 markers → averaged.
    std::map<std::pair<int,int>, std::pair<Point32f,int>> acc;  // (ic,ir) → (Σpos, n)
    for (const auto &m : ms) {
      Point32f mp[4]; const math::Homography2D H = localH(m.f, mp);
      Point32f pred[4]; std::pair<int,int> idx[4];
      for (int k = 0; k < 4; ++k) {
        pred[k] = H.apply(Point32f(mp[k].x/fill, mp[k].y/fill));
        const int q = (mp[k].x > 0 ? 2 : 0) | (mp[k].y > 0 ? 1 : 0);
        idx[k] = { m.cx + R[q].first, m.cy + R[q].second };
      }
      // snap tolerance: half the smallest gap between predicted corners (adapts to
      // the marker's apparent size/perspective, can't reach a neighbouring corner)
      float tol = 1e18f;
      for (int a = 0; a < 4; ++a)
        for (int b = a+1; b < 4; ++b)
          tol = std::min(tol, std::hypot(pred[a].x-pred[b].x, pred[a].y-pred[b].y));
      tol *= 0.5f;

      for (int k = 0; k < 4; ++k) {
        float best = tol; int bi = -1;
        for (size_t s = 0; s < seeds.size(); ++s) {
          const float d = std::hypot(pred[k].x-seeds[s].pos.x, pred[k].y-seeds[s].pos.y);
          if (d < best) { best = d; bi = (int)s; }
        }
        if (bi < 0) continue;
        auto &e = acc[idx[k]];
        e.first.x += seeds[bi].pos.x; e.first.y += seeds[bi].pos.y; ++e.second;
      }
    }

    // assemble the labelled corners into a (partial) inner-corner lattice and run
    // the same gradient sub-pixel polish CheckerboardTarget uses — the marker
    // anchoring got us the coarse saddle + its absolute (col,row) label; this pulls
    // each corner onto the true X-junction (raw saddles are only ~1px accurate).
    const int GC = D.cols-1, GR = D.rows-1;         // inner-corner lattice dims
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

    std::vector<CalibrationCorrespondence> out;
    out.reserve(grid.count);
    for (int ir = 0; ir < GR; ++ir)
      for (int ic = 0; ic < GC; ++ic)
        if (grid.filled[(size_t)ir*GC + ic])
          out.push_back({Vec(ic*sq, ir*sq, 0.f, 1.f), grid.at(ic, ir)});
    return out;
  }

} // namespace icl::markers
