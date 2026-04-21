// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/core/Image.h>

#include <string>

namespace icl::qt {

  /** @{ @name Image I/O (Quick2) */

  /// Write an image to disk
  ICLQt_API void save(const core::Image &image, const std::string &filename);

  /// Show an image using an external viewer
  ICLQt_API void show(const core::Image &image);

  /// Print image parameters to stdout
  ICLQt_API void print(const core::Image &image);

  /** @} */

  // tic/toc: identical signatures to old Quick — users get them from either header.
  // Not re-declared here to avoid duplicate symbols when both Quick.h and Quick2.h
  // coexist in the same library.

} // namespace icl::qt
