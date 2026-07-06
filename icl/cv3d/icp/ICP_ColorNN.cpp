// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Color-aware C++ nearest-neighbour backend for ICP: an exact brute-force NN
// under a weighted position+color metric. Colour disambiguates correspondences
// on geometrically ambiguous surfaces; it never enters the rigid-transform
// estimation (that stays position-only). Deliberately separate from OctreeNN so
// the default position-only Vec4 hot path carries no color branch.

#include <icl/cv3d/icp/ICP.h>
#include <icl/utils/Exception.h>

#include <limits>

namespace icl::cv3d {

  struct ColorNN::Data {
    icl32f colorWeight;
    std::vector<ICP::Vec> targetPos;
    std::vector<GeomColor> targetCol;   // parallel to targetPos (may be empty)
    std::vector<GeomColor> sourceCol;   // parallel to the query cloud (may be empty)
  };

  ColorNN::ColorNN(icl32f colorWeight) : m_data(new Data) {
    m_data->colorWeight = colorWeight;
  }
  ColorNN::~ColorNN() {}

  void ColorNN::setColorWeight(icl32f w) { m_data->colorWeight = w; }
  icl32f ColorNN::getColorWeight() const { return m_data->colorWeight; }

  void ColorNN::setTargetColors(const std::vector<GeomColor> &colors) {
    m_data->targetCol = colors;
  }
  void ColorNN::setSourceColors(const std::vector<GeomColor> &colors) {
    m_data->sourceCol = colors;
  }

  icl64f ColorNN::distanceSq(const ICP::Vec &qp, const GeomColor &qc,
                             const ICP::Vec &tp, const GeomColor &tc) const {
    const icl64f dx = qp[0]-tp[0], dy = qp[1]-tp[1], dz = qp[2]-tp[2];
    icl64f d = dx*dx + dy*dy + dz*dz;
    const icl64f w = m_data->colorWeight;
    if (w != 0.0) {
      const icl64f dr = qc[0]-tc[0], dg = qc[1]-tc[1], db = qc[2]-tc[2];
      d += w*w * (dr*dr + dg*dg + db*db);
    }
    return d;
  }

  void ColorNN::build(const std::vector<ICP::Vec> &target) {
    if (!m_data->targetCol.empty() && m_data->targetCol.size() != target.size()) {
      throw utils::ICLException("ColorNN::build: target color count != target point count");
    }
    m_data->targetPos = target;
  }

  void ColorNN::nearest(const std::vector<ICP::Vec> &queries,
                        std::vector<ICP::Vec> &out) const {
    out.resize(queries.size());
    const std::vector<ICP::Vec> &tp = m_data->targetPos;
    if (tp.empty()) {                 // build() never ran / empty target
      out = queries;
      return;
    }
    const bool haveTC = !m_data->targetCol.empty();
    const bool haveSC = !m_data->sourceCol.empty();
    if (haveSC && m_data->sourceCol.size() != queries.size()) {
      throw utils::ICLException("ColorNN::nearest: source color count != query count");
    }
    static const GeomColor kNoColor(0, 0, 0, 0);

    for (size_t i = 0; i < queries.size(); ++i) {
      const GeomColor &qc = haveSC ? m_data->sourceCol[i] : kNoColor;
      icl64f best = std::numeric_limits<icl64f>::max();
      size_t bi = 0;
      for (size_t j = 0; j < tp.size(); ++j) {
        const GeomColor &tc = haveTC ? m_data->targetCol[j] : kNoColor;
        const icl64f d = distanceSq(queries[i], qc, tp[j], tc);
        if (d < best) { best = d; bi = j; }
      }
      out[i] = tp[bi];
    }
  }

} // namespace icl::cv3d
