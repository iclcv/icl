// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/core/Image.h>

namespace icl::io {
  /// Minimal interface for image output classes
  /** The image output is used as generic interface for image sinks.
      Usually, it is recommended to use the GenericImageOutput class
      tha provides a string-based interface to set up the output
      backend.

      \section CMP Compression
      Outputs that need to compress before transmission (network sinks
      like WSImageOutput) own an `ImageCompressor` instance and expose
      its `Configurable` properties as a child under the `compression.`
      sub-scope, so callers can introspect or live-tune the codec via
      `Prop(&output)`. File-format outputs use the format's own
      compression mechanism and don't go through ImageCompressor.
  */
  struct ICLIO_API ImageOutput {
    /// virtual destructor
    virtual ~ImageOutput() {}

    /// ImageOutput instances must implement this method
    virtual void send(const core::Image &image) = 0;
  };
  } // namespace icl::io