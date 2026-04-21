// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Configurable.h>
#include <icl/utils/Time.h>
#include <icl/core/Image.h>
#include <icl/core/Img.h>
#include <string>

namespace icl::io {
  /// Pluggable image (de-)compression facade
  /** `ImageCompressor` is a thin facade over the `CompressionPlugin`
      registry. The sender picks a codec via `setCompression(...)`, calls
      `compress(image)` to get an envelope-wrapped byte view, and ships
      that. The receiver calls `uncompress(bytes, len)` and gets an
      `Image` back — the codec is auto-detected from the envelope, so
      the receiver does NOT need to be configured to match the sender.

      \section MODES Built-in plugins
      ICL ships with these plugins (see `CompressionPlugin*.cpp`):
        - `raw`  — planar concatenation, no transform (default; lossless,
                   any depth/channel count)
        - `rlen` — per-channel run-length encoding, icl8u only, four
                   quality levels (1/4/6/8 bits-per-value)
        - `jpeg` — libjpeg, lossy, icl8u only (built when libjpeg present)
        - `1611` — Kinect 11-bit pack for icl16s 1ch depth images
        - `zstd` — libzstd lossless general-purpose (built when libzstd present)

      Discover what's available at runtime via
      `CompressionRegister::names()`. Add a new codec by writing a single
      `CompressionPlugin*Foo*.cpp` and dropping `REGISTER_COMPRESSION_PLUGIN`
      at the bottom — no edits to this class required.

      \section WIRE Wire envelope
      Every encoded buffer starts with a fixed 46-byte binary prefix
      followed by variable-length codec name, codec params, image meta
      data, and finally the codec payload. See `ImageCompressor.cpp` for
      the byte-by-byte layout. The format intentionally breaks
      compatibility with the pre-Session 47 `Header::Params` POD
      (codec names are no longer 4-byte truncated; tunable codec
      parameters can be richer than `int32 quality`).
   */
  class ICLIO_API ImageCompressor : public utils::Configurable {
    struct Data;
    Data *m_data;

    public:
    ImageCompressor(const ImageCompressor&) = delete;
    ImageCompressor &operator=(const ImageCompressor&) = delete;


    /// Codec selection: name + opaque per-codec parameters string.
    /** `mode` is the codec name (matches a registered plugin). `quality`
        is the historical name for what is really an opaque per-codec
        parameter blob — most plugins interpret it as an integer
        (`jpeg` quality, `rlen` quality, `zstd` level), but the type is
        plugin-specific. */
    struct ICLIO_API CompressionSpec {
      explicit CompressionSpec(const std::string &mode    = "raw",
                               const std::string &quality = "")
        : mode(mode), quality(quality) {}
      std::string mode;
      std::string quality;
    };

    /// Non-owning view into the compressor's output buffer.
    /** Valid until the next call to `compress()` on the same compressor. */
    struct ICLIO_API CompressedData {
      CompressedData(icl8u *bytes = nullptr, int len = 0,
                     float compressionRatio = 0,
                     const CompressionSpec &compression = CompressionSpec())
        : bytes(bytes), len(len), compressionRatio(compressionRatio),
          compression(compression) {}
      icl8u *bytes;            ///< pointer into the compressor's buffer
      int    len;              ///< number of valid bytes
      float  compressionRatio; ///< encoded / raw, 1.0 = no compression
      CompressionSpec compression;
    };

    /// Construct with the given codec selection (default: `raw`).
    ImageCompressor(const CompressionSpec &spec = CompressionSpec());

    /// Destructor.
    virtual ~ImageCompressor();

    /// Switch the active codec (rebuilds the plugin instance).
    /** Throws if `spec.mode` is not a registered codec. */
    virtual void setCompression(const CompressionSpec &spec);

    /// Currently active codec selection.
    virtual CompressionSpec getCompression() const;

    /// Encode `img` into the envelope. Returns a non-owning view valid
    /// until the next `compress()` call. Throws on plugin failure.
    CompressedData compress(const core::Image &img, bool skipMetaData = false);

    /// Decode an envelope-wrapped buffer. The codec is read from the
    /// envelope, looked up in the registry, and used to decompress. The
    /// receiver's currently-set compression mode is ignored.
    core::Image uncompress(const icl8u *bytes, int len);

    /// Peek at the timestamp encoded in the envelope without doing the
    /// full decode (cheap; useful for staleness checks).
    utils::Time pickTimeStamp(const icl8u *bytes, int len);

    private:
    /// Replaces the active plugin and re-attaches it as a child Configurable.
    void installPlugin(const std::string &mode, const std::string &params);
  };
} // namespace icl::io
