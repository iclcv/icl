// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/filter/UnaryOp.h>
#include <icl/core/Image.h>

namespace icl::filter {

  /// UnaryOp computing a single gradient channel from an input image.
  /**
      Exposes the same four outputs as the older `GradientImage` helper class —
      x-gradient, y-gradient, intensity sqrt(gx²+gy²), and angle atan2(gy,gx) —
      but as a drop-in UnaryOp whose output channel is selected via the "mode"
      Configurable property, so it plugs straight into filter-playground and
      anywhere else that speaks UnaryOp.

      Internally driven by two Sobel-3x3 ConvolutionOps; angle/intensity are
      computed from the int16 X/Y buffers pixel-wise and returned as 32f.
  */
  class ICLFilter_API GradientOp : public UnaryOp {
    public:
    GradientOp(const GradientOp&) = delete;
    GradientOp& operator=(const GradientOp&) = delete;

    enum Mode { x, y, intensity, angle };

    explicit GradientOp(Mode mode = intensity);
    ~GradientOp() override;

    void apply(const core::Image &src, core::Image &dst) override;
    using UnaryOp::apply;

    Mode getMode() const { return m_mode; }
    void setMode(Mode m);

    bool getNormalize() const { return m_normalize; }
    void setNormalize(bool n);

    private:
    Mode m_mode;
    bool m_normalize;
    struct Data;
    Data *m_data;
  };

  } // namespace icl::filter
