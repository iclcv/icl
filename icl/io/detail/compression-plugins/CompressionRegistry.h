// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/plugin/PluginRegistry.h>
#include <icl/io/detail/compression-plugins/CompressionPlugin.h>

#include <functional>
#include <memory>

namespace icl::io {
  /// Process-wide registry of available `CompressionPlugin` factories.
  /** Alias over `utils::ClassPluginRegistry<CompressionPlugin>`. Plugins
      self-register at static-init time via `REGISTER_COMPRESSION_PLUGIN`
      (see below). Consumers use the registry directly:
      \code
        auto p = compressionRegistry().getOrThrow("jpeg").payload();
        p->prop("quality").value = "80";
        auto bytes = p->compress(image);

        for (const auto &name : compressionRegistry().keys()) { ... }
      \endcode

      Replaces the pre-Phase-6 `CompressionRegister` façade class. */
  using CompressionPluginRegistry = utils::ClassPluginRegistry<CompressionPlugin>;

  /// Singleton accessor for the process-wide compression registry.
  /// Throw-on-duplicate registration policy.
  ICLIO_API CompressionPluginRegistry& compressionRegistry();
} // namespace icl::io

/// Self-register a `CompressionPlugin` subclass at static-init time.
/** Use exactly once per plugin class, at the bottom of its `.cpp`:
    \code
      REGISTER_COMPRESSION_PLUGIN(zstd, [](){
        return std::unique_ptr<icl::io::CompressionPlugin>(new ZstdPlugin);
      });
    \endcode
    Wrap optional-dep plugins in the appropriate `#ifdef ICL_HAVE_*`. */
#define REGISTER_COMPRESSION_PLUGIN(NAME, FACTORY_LAMBDA)                      \
  ICL_REGISTER_PLUGIN(::icl::io::compressionRegistry(),                        \
                      NAME, #NAME, FACTORY_LAMBDA)
