// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/plugin/PluginRegistry.h>
#include <icl/utils/ProgArg.h>
#include <icl/utils/config/Configurable.h>
#include <icl/core/Image.h>

#include <memory>
#include <string>

namespace icl::io {
  class SinkBackend;  // detail/SinkBackend.h

  /// Process-wide registry of image-sink backends (type name → factory).
  /** The factory takes the `params` string, constructs the backend, and
      returns it as a `shared_ptr<SinkBackend>` (an object, not a bare
      callable — so its Configurable tunables survive and can be forwarded).
      Populated at static-init time by each backend's `.cpp` via
      `REGISTER_SINK_BACKEND`. */
  using SinkBackendRegistry =
      utils::FunctionPluginRegistry<std::shared_ptr<SinkBackend>(const std::string&)>;
  ICLIO_API SinkBackendRegistry& sinkBackendRegistry();

  /// Generic, string-configurable image sink (user-facing master type).
  /** The write-side counterpart to ImageSource: selects a backend by
      type+params string and forwards `send()` to it.

      ImageSink inherits Configurable and forwards the active backend's
      tunables as its own properties (via addChildConfigurable), so
      callers can do `sink.setPropertyValue("compression.mode", "jpeg")`
      or `sink.setPropertyValue("jpeg.quality", 85)` directly.  The
      previous std::function-based output master had no way to reach those.

      \section BACK Supported Backends

      Built-ins (always registered):
        - "null" — discards every frame
        - "file" (description=filepattern) — extension dispatches to the
                                             matching file-writer plugin

      Built conditionally on optional dependencies:
        - "ws"    (description=PORT or BIND:PORT) — WebSocket server
        - "video" (description=file,FOURCC,size,fps) — libav video writer

      Pass `-o list` (or type=="list") to print the up-to-date table.
  */
  class ICLIO_API ImageSink : public utils::Configurable {
    std::string type;
    std::string description;
    std::shared_ptr<SinkBackend> m_backend;

    public:

    /// Null constructor
    ImageSink(){}

    /// Create and initialize
    ImageSink(const std::string &type, const std::string &description);

    /// Create from given program argument
    ImageSink(const utils::ProgArg &pa);

    /// initialize this instance
    void init(const std::string &type, const std::string &description);

    /// initialization method (from given progarg)
    void init(const utils::ProgArg &pa);

    /// releases the backend (after this, isNull() returns true)
    void release();

    /// sends a new image
    void send(const core::Image &image);

    /// returns whether this instance was already initialized
    bool isNull() const { return !m_backend; }

    /// current type string
    const std::string &getType() const { return type; }

    /// current description string
    const std::string &getDescription() const { return description; }

    /// Escape hatch: the active backend as a Configurable (nullptr if none).
    /** Prefer the forwarded properties (setPropertyValue) over reaching in
        through this. */
    SinkBackend *getBackend() const { return m_backend.get(); }
  };
  } // namespace icl::io

/// Self-register an image-sink backend at static-init time.
/** The FACTORY expression must be a callable
    `(const std::string& params) -> std::shared_ptr<icl::io::SinkBackend>`.
    The Entry's description field doubles as a "paramHint~explanation" pair
    for the `-o list` affordance (separated by `~`). Example:
    \code
      REGISTER_SINK_BACKEND(ws, "ws",
        [](const std::string& p) -> std::shared_ptr<icl::io::SinkBackend> {
          return std::make_shared<WSImageOutput>(parsePort(p));
        },
        "PORT or BIND:PORT~WebSocket server")
    \endcode */
#define REGISTER_SINK_BACKEND(TAG, TYPE, FACTORY, HELP)                        \
  extern "C" __attribute__((constructor, used)) void                          \
  iclRegisterSinkBackend_##TAG() {                                            \
    ::icl::io::sinkBackendRegistry().registerPlugin((TYPE), FACTORY, HELP);   \
  }
