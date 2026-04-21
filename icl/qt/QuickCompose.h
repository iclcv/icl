// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/core/Image.h>

namespace icl::qt {

  /** @{ @name Image concatenation operators (Quick2) */

  /// Horizontal concatenation (a left, b right)
  ICLQt_API core::Image operator,(const core::Image &a, const core::Image &b);

  /// Vertical concatenation (a top, b bottom)
  ICLQt_API core::Image operator%(const core::Image &a, const core::Image &b);

  /// Channel concatenation (a's channels followed by b's channels)
  ICLQt_API core::Image operator|(const core::Image &a, const core::Image &b);

  /** @} */

  /** @{ @name Image ROI copy (Quick2) */

  /// Helper struct for ROI-based deep copy operations
  struct ICLQt_API ImgROI2 {
    core::Image image;

    /// Deep-copy i's ROI data into this image's ROI
    ImgROI2 &operator=(const core::Image &i);

    /// Fill this image's ROI with a constant value
    ImgROI2 &operator=(float val);

    /// Deep-copy another ROI into this image's ROI
    ImgROI2 &operator=(const ImgROI2 &r);

    /// Implicit conversion to Image
    operator core::Image();
  };

  /// Creates an ImgROI2 referencing the image's current ROI
  ICLQt_API ImgROI2 roi(core::Image &r);

  /// Creates an ImgROI2 referencing the full image (full ROI)
  ICLQt_API ImgROI2 data(core::Image &r);

  /** @} */

} // namespace icl::qt
