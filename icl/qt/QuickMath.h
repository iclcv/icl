// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/core/Image.h>

namespace icl::qt {

  /** @{ @name Image arithmetic operators (Quick2) */

  ICLQt_API core::Image operator+(const core::Image &a, const core::Image &b);
  ICLQt_API core::Image operator-(const core::Image &a, const core::Image &b);
  ICLQt_API core::Image operator*(const core::Image &a, const core::Image &b);
  ICLQt_API core::Image operator/(const core::Image &a, const core::Image &b);

  ICLQt_API core::Image operator+(const core::Image &image, float val);
  ICLQt_API core::Image operator-(const core::Image &image, float val);
  ICLQt_API core::Image operator*(const core::Image &image, float val);
  ICLQt_API core::Image operator/(const core::Image &image, float val);

  ICLQt_API core::Image operator+(float val, const core::Image &image);
  ICLQt_API core::Image operator-(float val, const core::Image &image);
  ICLQt_API core::Image operator*(float val, const core::Image &image);
  ICLQt_API core::Image operator/(float val, const core::Image &image);

  /// Unary negation
  ICLQt_API core::Image operator-(const core::Image &image);

  /** @} */

  /** @{ @name Image math functions (Quick2) */

  ICLQt_API core::Image exp(const core::Image &image);
  ICLQt_API core::Image ln(const core::Image &image);
  ICLQt_API core::Image sqr(const core::Image &image);
  ICLQt_API core::Image sqrt(const core::Image &image);
  ICLQt_API core::Image abs(const core::Image &image);

  /** @} */

  /** @{ @name Image logical operators (Quick2) */

  /// Pixel-wise logical OR (result: 255 if either > 0, else 0)
  ICLQt_API core::Image operator||(const core::Image &a, const core::Image &b);

  /// Pixel-wise logical AND (result: 255 if both > 0, else 0)
  ICLQt_API core::Image operator&&(const core::Image &a, const core::Image &b);

  /// Pixel-wise binary OR (values cast to T before operation)
  template<class T> ICLQt_API
  core::Image binOR(const core::Image &a, const core::Image &b);

  /// Pixel-wise binary XOR (values cast to T before operation)
  template<class T> ICLQt_API
  core::Image binXOR(const core::Image &a, const core::Image &b);

  /// Pixel-wise binary AND (values cast to T before operation)
  template<class T> ICLQt_API
  core::Image binAND(const core::Image &a, const core::Image &b);

  /** @} */

} // namespace icl::qt
