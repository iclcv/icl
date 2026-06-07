// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/config/Configurable.h>
#include <icl/core/Image.h>

namespace icl::io {
  /// Abstract image-sink backend (private — lives under detail/).
  /** The write-side counterpart to the source backend contract.  Each
      concrete sink (file, ws, video, null) derives this, inherits
      utils::Configurable so its tunables surface as properties, and
      implements send().

      `ImageSink` (the user-facing master) selects one of these by
      type+params string, holds it as a shared_ptr, and forwards both
      send() and — via addChildConfigurable — the backend's properties.
      That property forwarding is what lets callers do
      `sink.setPropertyValue("compression.mode", "jpeg")` directly, which
      the previous std::function-based output master could not. */
  class ICLIO_API SinkBackend : public utils::Configurable {
    public:
    virtual ~SinkBackend() = default;

    /// consume one image
    virtual void send(const core::Image &image) = 0;
  };
} // namespace icl::io
