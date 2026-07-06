// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// "nn.color" — color-aware C++ ICP backend: exact brute-force NN under a weighted
// position+color metric. Colour only disambiguates correspondences on
// geometrically ambiguous surfaces; it never enters the rigid-transform estimate.
// The protected distanceSq() is the free-form-metric seam — subclass here and
// register another name for a fully custom position+colour distance. Self-registers.

#include <icl/cv3d/icp/detail/ICPBackendRegistry.h>
#include <icl/utils/Exception.h>

#include <limits>
#include <cmath>
#include <memory>

namespace icl::cv3d {
  namespace {

    class ColorNN : public ICP::ColorBackend {
      icl32f m_colorWeight;
      std::vector<ICP::Vec> m_targetPos;
      std::vector<GeomColor> m_targetCol;   // parallel to m_targetPos (may be empty)
      std::vector<GeomColor> m_sourceCol;   // parallel to the query cloud (may be empty)

    public:
      explicit ColorNN(icl32f colorWeight = 1.0f) : m_colorWeight(colorWeight) {}

      void setColorWeight(icl32f w) override { m_colorWeight = w; }
      icl32f getColorWeight() const override { return m_colorWeight; }
      void setTargetColors(const std::vector<GeomColor> &c) override { m_targetCol = c; }
      void setSourceColors(const std::vector<GeomColor> &c) override { m_sourceCol = c; }

    protected:
      /// squared metric between a query (transformed source) and a target point.
      /** Default: `||Δpos||² + colorWeight²·||Δrgb||²`. Override (in a sibling
          detail backend) for a free-form position+color distance. */
      virtual icl64f distanceSq(const ICP::Vec &qp, const GeomColor &qc,
                                const ICP::Vec &tp, const GeomColor &tc) const {
        const icl64f dx = qp[0]-tp[0], dy = qp[1]-tp[1], dz = qp[2]-tp[2];
        icl64f d = dx*dx + dy*dy + dz*dz;
        const icl64f w = m_colorWeight;
        if (w != 0.0) {
          const icl64f dr = qc[0]-tc[0], dg = qc[1]-tc[1], db = qc[2]-tc[2];
          d += w*w * (dr*dr + dg*dg + db*db);
        }
        return d;
      }

    public:
      void build(const std::vector<ICP::Vec> &target) override {
        if (!m_targetCol.empty() && m_targetCol.size() != target.size()) {
          throw utils::ICLException("ColorNN::build: target color count != target point count");
        }
        m_targetPos = target;
      }

      void nearest(const std::vector<ICP::Vec> &queries,
                   std::vector<ICP::Vec> &out) const override {
        out.resize(queries.size());
        const std::vector<ICP::Vec> &tp = m_targetPos;
        if (tp.empty()) {                 // build() never ran / empty target
          out = queries;
          return;
        }
        const bool haveTC = !m_targetCol.empty();
        const bool haveSC = !m_sourceCol.empty();
        if (haveSC && m_sourceCol.size() != queries.size()) {
          throw utils::ICLException("ColorNN::nearest: source color count != query count");
        }
        static const GeomColor kNoColor(0, 0, 0, 0);

        for (size_t i = 0; i < queries.size(); ++i) {
          const GeomColor &qc = haveSC ? m_sourceCol[i] : kNoColor;
          icl64f best = std::numeric_limits<icl64f>::max();
          size_t bi = 0;
          for (size_t j = 0; j < tp.size(); ++j) {
            const GeomColor &tc = haveTC ? m_targetCol[j] : kNoColor;
            const icl64f d = distanceSq(queries[i], qc, tp[j], tc);
            if (d < best) { best = d; bi = j; }
          }
          out[i] = tp[bi];
        }
      }
    };

  } // anonymous namespace

  ICL_REGISTER_PLUGIN(icpBackendRegistry(), nn_color,
                      "nn.color",
                      [] { return std::static_pointer_cast<ICP::Backend>(
                                    std::make_shared<ColorNN>()); },
                      "C++ colour-aware (weighted position+colour) NN", /*priority*/ 5);

} // namespace icl::cv3d
