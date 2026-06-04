// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/plugin/PluginRegistry.h>
#include <icl/utils/ProgArg.h>
#include <icl/core/Image.h>

#include <functional>
#include <string>

namespace icl::io {
  /// Sender callable: receives one image and forwards it to a backend.
  /// Returned by each backend's factory after construction; holds backend
  /// state via captured `shared_ptr<ImplType>` closures.
  using ImageOutputFn = std::function<void(const core::Image&)>;

  /// Process-wide registry of image-output backends (type name → factory).
  /// The factory takes the `description`/`params` string, constructs the
  /// backend, and returns a sender callable. The registry is populated
  /// at static-init time by each backend's `.cpp` via `REGISTER_IMAGE_OUTPUT`.
  using ImageOutputRegistry = utils::FunctionPluginRegistry<ImageOutputFn(const std::string&)>;
  ICLIO_API ImageOutputRegistry& imageOutputRegistry();

  /// Generic Sink for images.
  /** Like the GenericGrabber, the GenericImageOutput provides a
      string-configurable interface for arbitrary image sinks.

      \section BACK Supported Backends

      Currently:
        - "file" (description=filepattern)
        - "video" (description=output-video-filename,CODEC-FOURCC=DIV3,VideoSize=VGA,FPS=24)
        - "ws"   (WebSocket server, description=PORT or BIND:PORT)
        - "v4l"  (V4L2 loopback device name)

      Pass `-o list` (or type=="list") to auto-print the up-to-date
      table from the process-wide registry.
  */
  class ICLIO_API GenericImageOutput {
    std::string   type;
    std::string   description;
    ImageOutputFn impl;

    public:

    /// Null constructor
    GenericImageOutput(){}

    /// Create and initialize
    /** @see init */
    GenericImageOutput(const std::string &type, const std::string &description);

    /// Create from given program argument
    GenericImageOutput(const utils::ProgArg &pa);

    /// initialize this instance
    void init(const std::string &type, const std::string &description);

    /// initialization method (from given progarg)
    void init(const utils::ProgArg &pa);

    /// releases the internal sender callable (after this, isNull() returns true)
    void release();

    /// sends a new image
    void send(const core::Image &image);

    /// returns whether this instance was already initialized
    bool isNull() const { return !impl; }

    /// current type string
    const std::string &getType() const { return type; }

    /// current description string
    const std::string &getDescription() const { return description; }

    // Compression configuration is exposed via the underlying output's
    // Configurable child (under the `compression.` prefix on outputs that
    // own an ImageCompressor — e.g. WSImageOutput). Access the backend
    // directly (not via GenericImageOutput) to read/write those properties.
  };
  } // namespace icl::io

/// Self-register an image-output backend at static-init time.
/** Expands to `ICL_REGISTER_PLUGIN`. The FACTORY expression must be a
    callable `(const std::string& params) -> ImageOutputFn`. The Entry's
    description field doubles as a "paramHint~explanation" pair for the
    `-o list` affordance (separated by `~`). Example:
    \code
      REGISTER_IMAGE_OUTPUT(ws, "ws",
        [](const std::string& p) -> icl::io::ImageOutputFn {
          auto impl = std::make_shared<WSImageOutput>(parsePort(p));
          return [impl](const core::Image& img) { impl->send(img); };
        },
        "PORT or BIND:PORT~WebSocket server")
    \endcode */
#define REGISTER_IMAGE_OUTPUT(TAG, TYPE, FACTORY, HELP)                        \
  extern "C" __attribute__((constructor, used)) void                           \
  iclRegisterImageOutput_##TAG() {                                             \
    ::icl::io::imageOutputRegistry().registerPlugin((TYPE), FACTORY, HELP);    \
  }
