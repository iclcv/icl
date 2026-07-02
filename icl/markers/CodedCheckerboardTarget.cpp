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
using namespace icl::geom;

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
    SquareBCHCode code;          ///< renders the marker patterns for generate()
    std::string detType;         ///< FiducialDetector plugin type

    /// a marker-bearing interior white cell (checker-cell coords)
    struct Coded { int id, cx, cy; };
    std::vector<Coded> coded;
    std::map<int, std::pair<int,int>> id2cell;   ///< marker id → (cx,cy)

    // detection state (lazy; mutated from the const detect())
    std::unique_ptr<FiducialDetector> fd;
    cv::CheckerboardSaddleDetector saddle;

    Data(int c, int r, float sq, float f, SquareBCHPreset p)
      : cols(c), rows(r), squareMM(sq), fill(f), preset(p),
        code(SquareBCHCode::presetInfo(p).gridSize, SquareBCHCode::presetInfo(p).correctable),
        detType(detectorTypeFor(p)) {
      // count the interior white cells (those with 4 surrounding inner corners):
      // cx∈[1,cols-2], cy∈[1,rows-2], white where (cx+cy) is even.
      std::vector<std::pair<int,int>> cells;
      for (int cy = 1; cy <= rows-2; ++cy)
        for (int cx = 1; cx <= cols-2; ++cx)
          if (((cx + cy) & 1) == 0) cells.push_back({cx, cy});

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
  };

  CodedCheckerboardTarget::CodedCheckerboardTarget(int cols, int rows, float squareSizeMM,
                                                   float markerFill, SquareBCHPreset preset)
    : m_data(new Data(cols, rows, squareSizeMM, markerFill, preset)) {}

  CodedCheckerboardTarget::~CodedCheckerboardTarget() { delete m_data; }

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

    // stamp a BCH marker into each coded white cell, centred and filling `fill` of
    // the cell so it stays clear of the cell edges (the checker corners survive).
    const int mpx = std::max(1, (int)std::lround(m_data->fill * px));
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
    const float sq = D.squareMM, fill = D.fill;

    // lazily build the BCH detector over exactly this board's marker ids
    if (!D.fd) {
      std::vector<int> ids; ids.reserve(D.coded.size());
      for (const auto &c : D.coded) ids.push_back(c.id);
      D.fd.reset(new FiducialDetector(D.detType, ids, ParamMap{{"size", Size(100,100)}}));
      // The checkerboard's own solid black squares get detected as quads; with
      // error correction the small n×n codes would spuriously decode such near-
      // uniform patches to a valid id. Require an EXACT decode (0 corrected
      // errors) so only genuine, cleanly-rendered markers are accepted.
      D.fd->getPlugin()->setPropertyValue("max bch errors", 0);
    }

    const std::vector<Fiducial> &fids = D.fd->detect(&image);
    if (fids.empty()) return {};
    const std::vector<cv::CornerSeed> seeds = D.saddle.detect(image);
    if (seeds.empty()) return {};

    // Corner labeling is split into POSITION and LABEL to be robust:
    //  * POSITION: a per-marker marker-local→image homography (from the marker's 4
    //    key points) predicts its 4 surrounding checker corners and snaps each to
    //    the nearest sub-pixel saddle. Local ⇒ accurate even under lens distortion.
    //  * LABEL: the marker id reliably gives its CELL, so a single global
    //    image→board homography (fit from all marker centres) turns a snapped
    //    saddle into its absolute (col,row) index by rounding. Global geometry ⇒
    //    independent of the per-marker orientation convention (which differs by
    //    code), needing only half-cell accuracy for the rounding.
    // A corner is shared by up to 4 markers → averaged.
    std::vector<Point32f> boardC, imgC;
    std::vector<const Fiducial*> markers;
    for (const Fiducial &f : fids) {
      auto itc = D.id2cell.find(f.getID());
      if (itc == D.id2cell.end()) continue;
      const std::vector<Fiducial::KeyPoint> &kps = f.getKeyPoints2D();
      if (kps.size() != 4) continue;
      Point32f c(0,0);
      for (int k = 0; k < 4; ++k) { c.x += kps[k].imagePos.x; c.y += kps[k].imagePos.y; }
      boardC.push_back(Point32f((itc->second.first - 0.5f)*sq, (itc->second.second - 0.5f)*sq));
      imgC.push_back(Point32f(c.x*0.25f, c.y*0.25f));
      markers.push_back(&f);
    }
    if (boardC.size() < 4) return {};                 // need enough for a homography
    const math::Homography2D Hib(boardC.data(), imgC.data(), (int)boardC.size());  // image → board

    std::map<std::pair<int,int>, std::pair<Point32f,int>> acc;  // (ic,ir) → (Σpos, n)
    for (const Fiducial *fp : markers) {
      const std::vector<Fiducial::KeyPoint> &kps = fp->getKeyPoints2D();
      Point32f mp[4], ip[4];
      for (int k = 0; k < 4; ++k) { mp[k] = kps[k].markerPos; ip[k] = kps[k].imagePos; }
      // ctor maps its 2nd arg → 1st, so (ip, mp) yields H.apply(markerPos) → image
      const math::Homography2D H(ip, mp, 4);          // marker-local mm → image px

      Point32f pred[4];
      for (int k = 0; k < 4; ++k) pred[k] = H.apply(Point32f(mp[k].x/fill, mp[k].y/fill));
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
        const Point32f b = Hib.apply(seeds[bi].pos);  // snapped saddle → board mm
        const std::pair<int,int> id{(int)std::lround(b.x/sq), (int)std::lround(b.y/sq)};
        auto &e = acc[id];
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
