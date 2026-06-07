// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

// Thin entry point for the two most common file-I/O operations.
// Wrappers over FileWriter / FileSource kept out-of-line so this header
// does not have to drag in either backend's headers — consumers that
// only need icl::io::save / icl::io::load pay near-zero include cost.

#include <icl/utils/CompatMacros.h>
#include <icl/core/Image.h>
#include <string>

namespace icl::io {

  /// Save an image to a file (extension determines the codec).
  /** Thin wrapper over FileWriter; null images are no-ops. */
  ICLIO_API void save(const core::Image &image, const std::string &filename);

  /// Load an image from a single file (extension determines the codec).
  /** Thin wrapper over FileSource.  Returns a null Image and logs an
      error if the file cannot be read.  For multi-file patterns or
      sequence iteration, use FileSource directly. */
  ICLIO_API core::Image load(const std::string &filename);

} // namespace icl::io
