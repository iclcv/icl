// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/filter/UnaryOp.h>
#include <icl/core/Converter.h>

namespace icl::filter {

  /// UnaryOp that converts images to fixed output parameters (size, format, depth)
  /** Wraps core::Converter to produce images with well-defined output
      parameters regardless of input. Useful for normalizing heterogeneous
      image sources to a common format.

      @see core::Converter
  */
  class ICLFilter_API FixedConvertOp : public UnaryOp {
  public:
    /// Creates a FixedConvertOp with given output parameters
    /** @param p output image parameters (size, channels, format)
        @param d output image depth
        @param applyToROIOnly if true, only the source ROI is converted */
    FixedConvertOp(const core::ImgParams &p, core::depth d = core::depth8u,
                   bool applyToROIOnly = false);

    /// Apply conversion
    void apply(const core::Image &src, core::Image &dst) override;

    /// Returns the fixed destination parameters (ignores source)
    std::pair<core::depth, core::ImgParams> getDestinationParams(const core::Image &src) const override;

    /// Sets output image parameters
    void setParams(const core::ImgParams &p) { m_params = p; }

    /// Returns current output parameters
    const core::ImgParams &getParams() const { return m_params; }

    /// Sets output depth
    void setDepth(core::depth d) { m_depth = d; }

    /// Returns current output depth
    core::depth getDepth() const { return m_depth; }

    /// Sets the operation order for internal conversion
    void setOperationOrder(core::Converter::oporder o) { m_converter.setOperationOrder(o); }

    /// Sets the scale interpolation mode (default: interpolateNN)
    void setScaleMode(core::scalemode sm) { m_converter.setScaleMode(sm); }

  private:
    core::ImgParams m_params;
    core::depth m_depth;
    core::Converter m_converter;
  };

} // namespace icl::filter
