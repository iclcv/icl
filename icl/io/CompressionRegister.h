// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Uncopyable.h>
#include <icl/io/CompressionPlugin.h>

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace icl::io {
  /// Process-wide registry of available `CompressionPlugin` factories.
  /** Mirrors `GrabberRegister` in shape. Plugins self-register at
      static-init time via `REGISTER_COMPRESSION_PLUGIN` (see below).

      \section USE Use
      \code
        // Sender side
        auto p = CompressionRegister::create("jpeg");
        p->setPropertyValue("quality", "80");
        auto bytes = p->compress(image);          // → wire envelope by ImageCompressor

        // Receiver side (codec name comes from the envelope)
        auto p = CompressionRegister::create(envelope.codecName);
        p->setCodecParamsString(envelope.codecParams);
        Image img = p->decompress({envelope.payload, envelope.payloadLen},
                                  envelope.params, envelope.depth);

        // Discovery
        for (const auto &name : CompressionRegister::names()) { ... }
      \endcode
   */
  class ICLIO_API CompressionRegister : public utils::Uncopyable {
    public:

    /// Factory: returns a fresh plugin instance owned by the caller.
    using Factory = std::function<std::unique_ptr<CompressionPlugin>()>;

    /// Singleton accessor.
    static CompressionRegister &instance();

    /// Register a codec. Throws if `name` is already registered.
    void registerPlugin(const std::string &name, Factory factory);

    /// Construct a fresh plugin instance for `name`. Throws if not registered.
    static std::unique_ptr<CompressionPlugin> create(const std::string &name);

    /// All registered codec names, sorted lexicographically. Useful for
    /// auto-populating the `compression` menu in `WSImageOutput` etc.
    static std::vector<std::string> names();

    /// Whether a codec is registered.
    static bool has(const std::string &name);

    private:
    CompressionRegister() = default;

    mutable std::mutex m_mutex;
    std::map<std::string, Factory, std::less<>> m_factories;
  };
} // namespace icl::io

/// Self-register a `CompressionPlugin` subclass at static-init time.
/** Use exactly once per plugin class, at the bottom of its `.cpp`:
    \code
      REGISTER_COMPRESSION_PLUGIN(zstd, [](){
        return std::unique_ptr<icl::io::CompressionPlugin>(new ZstdPlugin);
      });
    \endcode
    Wrap optional-dep plugins in the appropriate `#ifdef ICL_HAVE_*`. */
/* `__attribute__((constructor))` registers the function in the
 * dylib's __mod_init_func / .init_array section. The dynamic loader
 * walks that section on dlopen, so the function survives dead-stripping
 * regardless of whether anything else references it. (Plain anonymous-
 * namespace static-storage objects do NOT survive macOS dead_strip when
 * the rest of the TU is otherwise unreferenced.) The function name is
 * macro-mangled to be unique within the TU. */
#define REGISTER_COMPRESSION_PLUGIN(NAME, FACTORY_LAMBDA)                      \
  extern "C" __attribute__((constructor, used)) void                           \
  iclRegisterCompressionPlugin_##NAME() {                                      \
    ::icl::io::CompressionRegister::instance().registerPlugin(                 \
      #NAME, FACTORY_LAMBDA);                                                  \
  }
