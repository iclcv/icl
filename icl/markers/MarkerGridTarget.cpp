// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/markers/MarkerGridTarget.h>
#include <icl/markers/AdvancedMarkerGridDetector.h>
#include <icl/markers/FiducialDetector.h>
#include <icl/cv/SubPixelCornerRefiner.h>
#include <algorithm>
#include <cmath>
#include <memory>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::geom;

namespace icl::markers {

  typedef AdvancedMarkerGridDetector AMGD;

  struct MarkerGridTarget::Data {
    AMGD::AdvancedGridDefinition def;
    AMGD detector;        // detect() mutates its internal grid (PIMPL hides the non-const)
    AMGD::MarkerGrid model;   // reference grid (grid-space corner points) for modelPoints/generate
    bool subPixel = true;     // sub-pixel-refine the marker corners in detect()
  };

  MarkerGridTarget::MarkerGridTarget(const Size &numCells, const Size32f &markerBoundsMM,
                                     const Size32f &gridBoundsMM, const std::vector<int> &markerIDs,
                                     const std::string &markerType)
    : m_data(new Data) {
    std::vector<int> ids = markerIDs;
    if (ids.empty()) {                       // default IDs 0..N-1 (row-major)
      ids.resize((size_t)numCells.getDim());
      for (size_t i = 0; i < ids.size(); ++i) ids[i] = (int)i;
    }
    m_data->def = AMGD::AdvancedGridDefinition(numCells, markerBoundsMM, gridBoundsMM, ids, markerType);
    m_data->detector.init(m_data->def);
    m_data->model.init(m_data->def);
  }

  MarkerGridTarget::~MarkerGridTarget() { delete m_data; }

  void MarkerGridTarget::setSubPixelRefine(bool on) { m_data->subPixel = on; }
  bool MarkerGridTarget::getSubPixelRefine() const { return m_data->subPixel; }

  std::vector<CalibrationCorrespondence>
  MarkerGridTarget::detect(const core::Img8u &image) const {
    std::vector<CalibrationCorrespondence> out;
    const AMGD::MarkerGrid &g = m_data->detector.detect(&image);
    // The marker corners come from a thresholded binary region (~1px accurate).
    // Refine each marker's 4 corners to sub-pixel against the grayscale image
    // (border edge-line fit + intersection) — the dominant calibration error.
    std::unique_ptr<cv::SubPixelCornerRefiner> refiner;
    if (m_data->subPixel) refiner.reset(new cv::SubPixelCornerRefiner(image));
    for (auto it = g.begin(); it != g.end(); ++it) {
      const AMGD::Marker &m = *it;
      if (!m.wasFound()) continue;
      std::vector<Point32f> mp, ip;
      m.getGridPoints().appendCornersTo(mp);   // grid-space mm
      m.getImagePoints().appendCornersTo(ip);  // detected image px (cyclic order)
      if (refiner && ip.size() == 4) refiner->refineQuad(ip.data());
      for (size_t k = 0; k < mp.size() && k < ip.size(); ++k)
        out.push_back({Vec(mp[k].x, mp[k].y, 0.f, 1.f), ip[k]});
    }
    return out;
  }

  std::vector<Vec> MarkerGridTarget::modelPoints() const {
    std::vector<Vec> pts;
    for (auto it = m_data->model.begin(); it != m_data->model.end(); ++it) {
      std::vector<Point32f> mp;
      it->getGridPoints().appendCornersTo(mp);
      for (const auto &p : mp) pts.push_back(Vec(p.x, p.y, 0.f, 1.f));
    }
    return pts;
  }

  core::Img8u MarkerGridTarget::generate(const utils::Size &pixelSize) const {
    Img8u img(pixelSize, 1);
    img.fill(255);                              // white background / quiet zone
    Channel8u d = img[0];

    // uniform mm→px scale fitting the whole grid into pixelSize with a margin
    const Size32f gb = m_data->def.getGridBounds();
    if (!(gb.width > 0) || !(gb.height > 0)) return img;
    const float margin = 0.05f;
    const float s = std::min(pixelSize.width  * (1 - 2*margin) / gb.width,
                             pixelSize.height * (1 - 2*margin) / gb.height);
    const float ox = (pixelSize.width  - gb.width  * s) / 2.f;
    const float oy = (pixelSize.height - gb.height * s) / 2.f;

    FiducialDetector fd(m_data->def.getMarkerType(), ParamMap{{"size", "1x1"}});
    const std::vector<int> &ids = m_data->def.getMarkerIDs();
    const int W = m_data->def.getWidth(), H = m_data->def.getHeight();
    for (int y = 0, idx = 0; y < H; ++y)
      for (int x = 0; x < W; ++x, ++idx) {
        const Rect32f r = m_data->def.getBounds(x, y);            // mm
        const int px = (int)std::lround(ox + r.x * s), py = (int)std::lround(oy + r.y * s);
        const int pw = (int)std::lround(r.width * s), ph = (int)std::lround(r.height * s);
        if (pw < 1 || ph < 1) continue;
        const Img8u marker = fd.createMarker(ids[(size_t)idx], Size(pw, ph),
                                             ParamMap{{"border width", 2}});
        const Channel8u md = marker[0];
        const int mw = marker.getWidth(), mh = marker.getHeight();
        for (int j = 0; j < mh; ++j) {
          const int yy = py + j; if (yy < 0 || yy >= pixelSize.height) continue;
          for (int i = 0; i < mw; ++i) {
            const int xx = px + i; if (xx < 0 || xx >= pixelSize.width) continue;
            d(xx, yy) = md(i, j);
          }
        }
      }
    return img;
  }

} // namespace icl::markers
