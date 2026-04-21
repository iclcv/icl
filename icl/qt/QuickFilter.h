// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/core/Image.h>
#include <icl/core/Types.h>

#include <string>

namespace icl::qt {

  /** @{ @name Image filter functions (Quick2)
      All functions preserve the input image's depth unless noted otherwise. */

  /// Applies a named filter: sobelx, sobely, gauss, laplace, median, dilation, erosion, opening, closing
  ICLQt_API core::Image filter(const core::Image &image, const std::string &filter);

  /// Gaussian blur with given mask radius (kernel size = 2*maskRadius+1)
  ICLQt_API core::Image blur(const core::Image &image, int maskRadius = 1);

  /// Color-space conversion
  ICLQt_API core::Image cc(const core::Image &image, core::format fmt);

  /// Convert to RGB
  ICLQt_API core::Image rgb(const core::Image &image);

  /// Convert to HLS
  ICLQt_API core::Image hls(const core::Image &image);

  /// Convert to LAB
  ICLQt_API core::Image lab(const core::Image &image);

  /// Convert to grayscale
  ICLQt_API core::Image gray(const core::Image &image);

  /// Scale image by a factor
  ICLQt_API core::Image scale(const core::Image &image, float factor);

  /// Scale image to a given size
  ICLQt_API core::Image scale(const core::Image &image, int width, int height);

  /// Extract a single channel
  ICLQt_API core::Image channel(const core::Image &image, int channel);

  /// Reduce quantisation levels (converts via 8u internally)
  ICLQt_API core::Image levels(const core::Image &image, icl8u levels);

  /// Binary threshold: pixels > threshold become 255, others become 0
  ICLQt_API core::Image thresh(const core::Image &image, float threshold);

  /// Deep copy
  ICLQt_API core::Image copy(const core::Image &image);

  /// Deep copy of just the ROI
  ICLQt_API core::Image copyroi(const core::Image &image);

  /// Normalize all channels to [0, 255]
  ICLQt_API core::Image norm(const core::Image &image);

  /// Rotate image by given angle in degrees (clockwise, bilinear interpolation)
  ICLQt_API core::Image rotate(const core::Image &image, float angleDeg);

  /// Horizontal flip (mirror along vertical axis)
  ICLQt_API core::Image flipx(const core::Image &image);

  /// Vertical flip (mirror along horizontal axis)
  ICLQt_API core::Image flipy(const core::Image &image);

  /** @} */

} // namespace icl::qt
