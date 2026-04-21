// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Configurable.h>
#include <icl/core/Image.h>
#include <icl/core/ImgParams.h>
#include <icl/core/Types.h>
#include <cstddef>
#include <string>

namespace icl::io {
  /// Pluggable image-compression backend
  /** A `CompressionPlugin` encodes and decodes a single named codec
      (`raw`, `rlen`, `jpeg`, `1611`, `zstd`, …). Plugins are looked up
      by name through `CompressionRegister`; ICL's `ImageCompressor` is a
      thin facade that handles the wire envelope (codec name + per-codec
      params + image meta + image params) and delegates the actual
      compress/decompress to the registered plugin matching the active
      codec name. The receiver auto-detects the codec from the envelope
      and does NOT have to be configured to match the sender.

      \section CFG  Per-codec parameters
      Plugins inherit `utils::Configurable`, so each can expose its own
      tunables (`level`, `quality`, `mask bits`, …) with auto-UI via
      `qt::Prop(plugin)`. The wire envelope carries the codec's
      parameters as an opaque string (set/getCodecParamsString) so the
      receiver can reconstruct the encode-time settings if it cares (most
      plugins decode regardless of encode-time params).

      \section OWN  Bytes ownership
      `Bytes` is a non-owning view both directions:
        - `compress()` return: plugin owns storage; valid until the next
          `compress()` on the same plugin.
        - `decompress()` input: caller owns storage.

      \section REG  Registration
      Plugins self-register at static-init time via the
      `REGISTER_COMPRESSION_PLUGIN` macro in `CompressionRegister.h`.
   */
  class ICLIO_API CompressionPlugin : public utils::Configurable {
    public:

    virtual ~CompressionPlugin() = default;

    /// Codec name as it appears in the wire envelope, e.g. "zstd".
    /** Must be stable across runs and unique within the registry. */
    virtual std::string name() const = 0;

    /// Non-owning view into a byte buffer.
    struct Bytes {
      const icl8u *data;
      std::size_t  len;
    };

    /// Encode `src`. Result is a view into the plugin's internal buffer,
    /// valid until the next `compress()` call on the same plugin.
    virtual Bytes compress(const core::Image &src) = 0;

    /// Decode `bytes` into an `Image` shaped like (`params`, `d`). The
    /// facade has already parsed the envelope and provides the target
    /// shape so each plugin only deals with its own raw payload.
    virtual core::Image decompress(Bytes bytes,
                                   const core::ImgParams &params,
                                   core::depth d) = 0;

    /// Serialize the plugin's current parameters into the opaque string
    /// stored in the wire envelope. Default: empty string. Plugins with
    /// tunable parameters override (e.g. the zstd plugin returns its
    /// `level`).
    virtual std::string getCodecParamsString() const { return {}; }

    /// Apply parameters that came in on the wire envelope. Default: no
    /// op. Most decoders are independent of encode-time parameters; the
    /// hook exists for codecs whose decode path actually needs them
    /// (e.g. rlen quality picks a different unpack routine).
    virtual void setCodecParamsString(const std::string &/*params*/) {}
  };
} // namespace icl::io
