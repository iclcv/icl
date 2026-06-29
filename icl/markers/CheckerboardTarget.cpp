// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/markers/CheckerboardTarget.h>
#include <icl/cv/CheckerboardDetector.h>
#include <icl/cv/NativeCheckerboardDetector.h>
#include <icl/cv/CheckerboardGrid.h>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::geom;

namespace icl::markers {

  CheckerboardTarget::CheckerboardTarget(int cols, int rows, float squareSizeMM)
    : m_cols(cols), m_rows(rows), m_squareMM(squareSizeMM),
      m_detector(std::make_shared<cv::NativeCheckerboardDetector>()) {}

  void CheckerboardTarget::setDetector(std::shared_ptr<cv::CheckerboardDetector> detector) {
    m_detector = std::move(detector);
  }

  std::vector<Vec> CheckerboardTarget::modelPoints() const {
    std::vector<Vec> pts;
    pts.reserve((size_t)(m_cols-1)*(m_rows-1));
    for (int r = 0; r < m_rows-1; ++r)
      for (int c = 0; c < m_cols-1; ++c)
        pts.push_back(Vec(c*m_squareMM, r*m_squareMM, 0.f, 1.f));
    return pts;
  }

  std::vector<CalibrationCorrespondence>
  CheckerboardTarget::detect(const core::Img8u &image) const {
    std::vector<CalibrationCorrespondence> out;
    const int IC = m_cols-1, IR = m_rows-1;   // inner-corner lattice dimensions
    if (IC < 1 || IR < 1) return out;

    cv::CheckerboardDetector::Hints hints;
    hints.boardCells = Size(IC, IR);   // some backends (opencv) require it; native ignores it
    const cv::CheckerboardDetector::Result res = m_detector->detect(image, hints);
    if (res.empty()) return out;
    const cv::CheckerboardGrid &g = res.boards.front();

    // v1: require the full lattice, matching the board in either axis order. The
    // recovered (col,row) origin/orientation is arbitrary; we map it onto the
    // model enumeration so each view's correspondences are self-consistent.
    const bool direct    = (g.cols == IC && g.rows == IR);
    const bool transposed= (g.cols == IR && g.rows == IC);
    if (!g.complete() || (!direct && !transposed)) return out;

    out.reserve((size_t)IC*IR);
    for (int gr = 0; gr < g.rows; ++gr)
      for (int gc = 0; gc < g.cols; ++gc) {
        // model (col,row): same as grid when direct, swapped when transposed
        const int mc = direct ? gc : gr;
        const int mr = direct ? gr : gc;
        out.push_back({Vec(mc*m_squareMM, mr*m_squareMM, 0.f, 1.f), g.at(gc, gr)});
      }
    return out;
  }

  core::Img8u CheckerboardTarget::generate(const utils::Size &pixelSize) const {
    // checker squares + a 1-square white quiet zone, centred and isotropic so the
    // generated board is itself cleanly detectable (round-trips through detect()).
    Img8u img(pixelSize, 1);
    Channel8u d = img[0];
    const float px = std::min(pixelSize.width  / float(m_cols + 2),
                              pixelSize.height / float(m_rows + 2));   // px per square
    const float bw = px*m_cols, bh = px*m_rows;
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
    return img;
  }

} // namespace icl::markers
