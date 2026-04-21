// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/filter/UnaryOp.h>
#include <icl/core/Color.h>

namespace icl::filter {

  /// Converts a single-channel image to an RGB pseudo-color image via LUT
  /** Maps each pixel value through a 3-channel lookup table to produce a
      false-color visualization. Supports a built-in default gradient and
      custom piecewise-linear gradients defined by color stops.

      Output is always depth8u / formatRGB.

      @see core::Color
  */
  class ICLFilter_API PseudoColorOp : public UnaryOp {
  public:

    /// Color table mode
    enum ColorTable {
      Default, //!< built-in blue→green→red→yellow gradient
      Custom   //!< user-defined piecewise-linear gradient
    };

    /// A color stop for custom gradients
    struct Stop {
      Stop(float relPos = 0, const core::Color &color = core::Color(0,0,0))
        : relPos(relPos), color(color) {}
      float relPos;    //!< relative position in [0,1]
      core::Color color; //!< color at this position
    };

    using UnaryOp::apply;

    /// Creates a PseudoColorOp with the default color table
    /** @param maxValue maximum expected pixel value (default 255) */
    PseudoColorOp(int maxValue = 255);

    /// Creates a PseudoColorOp with a custom gradient
    /** @param stops at least one stop; auto-padded to [0,1] range
        @param maxValue maximum expected pixel value */
    PseudoColorOp(const std::vector<Stop> &stops, int maxValue = 255);

    /// Changes the color table at runtime
    /** In Custom mode, stops must contain at least one entry.
        Stops are sorted by relPos; missing 0.0/1.0 endpoints are
        auto-inserted (black/white). */
    void setColorTable(ColorTable t,
                       const std::vector<Stop> &stops = {},
                       int maxValue = 255);

    /// Apply the pseudo-color mapping
    void apply(const core::Image &src, core::Image &dst) override;

    /// Destination is always depth8u / formatRGB / same size as source
    std::pair<core::depth, core::ImgParams>
    getDestinationParams(const core::Image &src) const override;

    /// Save current stop configuration to XML file
    void save(const std::string &filename);

    /// Load stop configuration from XML file
    void load(const std::string &filename);

  private:
    struct Data;
    Data *m_data;
  };

} // namespace icl::filter
