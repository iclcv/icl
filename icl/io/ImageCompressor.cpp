// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/io/ImageCompressor.h>
#include <icl/utils/prop/Constraints.h>
#include <icl/io/detail/compression-plugins/CompressionPlugin.h>
#include <icl/io/detail/compression-plugins/CompressionRegistry.h>

#include <algorithm>
#include <icl/utils/Exception.h>
#include <icl/utils/StringUtils.h>
#include <icl/core/CoreFunctions.h>

#include <cstring>
#include <cstdint>
#include <memory>
#include <vector>

// ----------------------------------------------------------------------
// Force-link the built-in compression plugins. The plugins self-register
// via `__attribute__((constructor))` functions, but on macOS the linker
// drops the plugin .o entirely if NOTHING in the .o is externally
// referenced — its constructor function never makes it into the dylib's
// `__init_offsets` section, so dyld never calls it. Taking the address
// of each plugin's registration function below adds an external
// reference, so the .o stays linked and its constructor fires at
// dlopen-time as intended. Adding a new plugin = one extra entry here
// (until we move to a meson static-archive `link_whole` setup).
// ----------------------------------------------------------------------
extern "C" {
  void iclRegisterCompressionPlugin_raw();
  void iclRegisterCompressionPlugin_rlen();
  void iclRegisterCompressionPlugin_jpeg();
  void iclRegisterCompressionPlugin_1611();
#ifdef ICL_HAVE_ZSTD
  void iclRegisterCompressionPlugin_zstd();
#endif
}

namespace {
  [[maybe_unused]] void (*const iclForceLinkCompressionPlugins[])() = {
    &iclRegisterCompressionPlugin_raw,
    &iclRegisterCompressionPlugin_rlen,
    &iclRegisterCompressionPlugin_jpeg,
    &iclRegisterCompressionPlugin_1611,
#ifdef ICL_HAVE_ZSTD
    &iclRegisterCompressionPlugin_zstd,
#endif
  };
}

namespace icl::io {
  using namespace icl::utils;
  using namespace icl::core;

  // ----------------------------------------------------------------------
  // Wire envelope (little-endian; ICL targets ARM64 + x86_64, both LE):
  //
  //   offset  size  field
  //   ------  ----  ----------------------------------------------------
  //        0    4   magic = 'I''C''L''C'
  //        4    2   version = 1                          (uint16)
  //        6    2   flags (reserved, 0)                  (uint16)
  //        8    4   width                                (int32)
  //       12    4   height                               (int32)
  //       16    4   channels                             (int32)
  //       20    1   depth (encoded as core::depth enum)  (uint8)
  //       21    1   format (encoded as core::format enum)(uint8)
  //       22    4   roi_x                                (int32)
  //       26    4   roi_y                                (int32)
  //       30    4   roi_w                                (int32)
  //       34    4   roi_h                                (int32)
  //       38    8   timestamp_us                         (int64)
  //       46    2   codec_name_len (N)                   (uint16)
  //       48    N   codec_name                           (no \0 terminator)
  //     46+N+0  2   codec_params_len (M)                 (uint16)
  //     46+N+2  M   codec_params
  //  46+N+M+4  4   meta_len (K)                         (uint32)
  //  46+N+M+4  K   meta
  //                payload follows: rest of the buffer
  //
  // No padding — every field is byte-packed and read via memcpy. We
  // intentionally break the pre-Session-47 wire format (Header::Params
  // POD) so codec names can be longer than 4 chars and per-codec params
  // can be richer than `int32 quality`.
  // ----------------------------------------------------------------------

  namespace {
    constexpr int kFixedPrefix = 46;
    constexpr char kMagic[4] = {'I','C','L','C'};
    constexpr uint16_t kVersion = 1;

    template <typename T> void writeLE(icl8u *&p, T v) {
      std::memcpy(p, &v, sizeof(T));
      p += sizeof(T);
    }
    template <typename T> T readLE(const icl8u *&p) {
      T v;
      std::memcpy(&v, p, sizeof(T));
      p += sizeof(T);
      return v;
    }

    // Per-image envelope contents (everything except the codec payload).
    struct EnvelopeFields {
      ImgParams   params;
      depth       d;
      Time        timestamp;
      std::string codecName;
      std::string codecParams;
      std::string meta;
    };

    int envelopeSize(const EnvelopeFields &f) {
      return kFixedPrefix
           + 2 + static_cast<int>(f.codecName.size())
           + 2 + static_cast<int>(f.codecParams.size())
           + 4 + static_cast<int>(f.meta.size());
    }

    void writeEnvelope(icl8u *dst, const EnvelopeFields &f) {
      icl8u *p = dst;
      std::memcpy(p, kMagic, 4); p += 4;
      writeLE<uint16_t>(p, kVersion);
      writeLE<uint16_t>(p, 0);  // flags
      writeLE<int32_t>(p, f.params.getSize().width);
      writeLE<int32_t>(p, f.params.getSize().height);
      writeLE<int32_t>(p, f.params.getChannels());
      writeLE<uint8_t>(p, static_cast<uint8_t>(f.d));
      writeLE<uint8_t>(p, static_cast<uint8_t>(f.params.getFormat()));
      writeLE<int32_t>(p, f.params.getROI().x);
      writeLE<int32_t>(p, f.params.getROI().y);
      writeLE<int32_t>(p, f.params.getROI().width);
      writeLE<int32_t>(p, f.params.getROI().height);
      writeLE<int64_t>(p, f.timestamp.toMicroSeconds());
      writeLE<uint16_t>(p, static_cast<uint16_t>(f.codecName.size()));
      std::memcpy(p, f.codecName.data(), f.codecName.size()); p += f.codecName.size();
      writeLE<uint16_t>(p, static_cast<uint16_t>(f.codecParams.size()));
      std::memcpy(p, f.codecParams.data(), f.codecParams.size()); p += f.codecParams.size();
      writeLE<uint32_t>(p, static_cast<uint32_t>(f.meta.size()));
      std::memcpy(p, f.meta.data(), f.meta.size()); p += f.meta.size();
    }

    /// Returns the byte offset where the codec payload starts. Throws
    /// on malformed envelopes (bad magic, truncation, etc.).
    int parseEnvelope(const icl8u *bytes, int len, EnvelopeFields &out) {
      if (len < kFixedPrefix) {
        throw ICLException("ImageCompressor::parseEnvelope: buffer too short for fixed prefix");
      }
      if (std::memcmp(bytes, kMagic, 4) != 0) {
        throw ICLException("ImageCompressor::parseEnvelope: bad magic (not an ICL envelope)");
      }
      const icl8u *p = bytes + 4;
      const uint16_t version = readLE<uint16_t>(p);
      if (version != kVersion) {
        throw ICLException("ImageCompressor::parseEnvelope: unsupported version " + str(version));
      }
      readLE<uint16_t>(p);  // flags (ignored for now)

      const int32_t w  = readLE<int32_t>(p);
      const int32_t h  = readLE<int32_t>(p);
      const int32_t ch = readLE<int32_t>(p);
      const uint8_t d  = readLE<uint8_t>(p);
      const uint8_t fm = readLE<uint8_t>(p);
      const int32_t rx = readLE<int32_t>(p);
      const int32_t ry = readLE<int32_t>(p);
      const int32_t rw = readLE<int32_t>(p);
      const int32_t rh = readLE<int32_t>(p);
      const int64_t ts = readLE<int64_t>(p);

      out.params = ImgParams(Size(w, h), ch, static_cast<format>(fm), Rect(rx, ry, rw, rh));
      out.d         = static_cast<depth>(d);
      out.timestamp = Time(ts);

      auto readVarLen = [&](auto lenT, std::string &dst) {
        using L = decltype(lenT);
        if (p + sizeof(L) > bytes + len) {
          throw ICLException("ImageCompressor::parseEnvelope: truncated (length field)");
        }
        const L l = readLE<L>(p);
        if (p + l > bytes + len) {
          throw ICLException("ImageCompressor::parseEnvelope: truncated (string body)");
        }
        dst.assign(reinterpret_cast<const char*>(p), l);
        p += l;
      };
      readVarLen(uint16_t{}, out.codecName);
      readVarLen(uint16_t{}, out.codecParams);
      readVarLen(uint32_t{}, out.meta);

      return static_cast<int>(p - bytes);  // payload offset
    }
  } // anonymous namespace

  // ------------------------------------------------------- pimpl --
  struct ImageCompressor::Data {
    CompressionSpec                   spec;
    std::unique_ptr<CompressionPlugin> plugin;
    std::vector<icl8u>                 envelopeBuf;  // full envelope + payload concatenated

    Image                              decoded;       // last successful decode (kept alive
                                                     // for caller's pointer stability)
    std::unique_ptr<CompressionPlugin> decodePlugin;  // dispatched per-message; cached if
                                                     // the codec didn't change between calls
    std::string                        decodePluginName;
  };

  // -------------------------------------------------------- public --
  // The ctor installs the active plugin eagerly. This is safe because
  // FileWriter and FileGrabber are now factory-based (Session 47): no
  // FileWriterPluginBICL / FileGrabberPluginBICL instances are
  // constructed during static init — they're built lazily on first
  // use, after main() is running and the compressionRegistry() has been
  // fully populated. Anyone constructing an ImageCompressor outside of
  // a static initializer will see a fully-wired plugin immediately.
  ImageCompressor::ImageCompressor(const CompressionSpec &spec)
    : m_data(new Data) {
    m_data->spec = spec;
    // Build the codec menu from the (now-populated) plugin registry.
    // Historical lex-sort preserved for deterministic menu ordering.
    auto names = compressionRegistry().keys();
    std::sort(names.begin(), names.end());
    std::string menu;
    bool first = true;
    for (const auto &n : names) {
      if (!first) menu += ',';
      menu += n;
      first = false;
    }
    addProperty("mode", utils::prop::menuFromCsv(menu), spec.mode, 0,
                "Active codec. The receiver auto-detects the codec from "
                "the envelope, so it does NOT need to match this setting. "
                "Each codec exposes its own tunables as sibling properties; "
                "the set of siblings changes when `mode` changes.");
    Configurable::registerCallback([this](const Property &p){
      if (p.name == "mode") installPlugin(p.as<std::string>(), "");
    });
    installPlugin(spec.mode, spec.quality);
  }

  ImageCompressor::~ImageCompressor() {
    if (m_data->plugin) removeChildConfigurable(m_data->plugin.get());
    delete m_data;
  }

  void ImageCompressor::installPlugin(const std::string &mode,
                                      const std::string &params) {
    if (m_data->plugin) {
      removeChildConfigurable(m_data->plugin.get());
    }
    m_data->plugin = compressionRegistry().getOrThrow(mode).payload();
    if (!params.empty()) m_data->plugin->setCodecParamsString(params);
    // Add as child with empty prefix so the plugin's own properties
    // (`quality`, `level`, …) surface as siblings of `mode`. When this
    // ImageCompressor is itself a child of (e.g.) WSImageOutput under
    // the prefix `compression.`, the user sees `compression.mode` plus
    // `compression.quality` / `compression.level` etc. — the codec's
    // tunables appear and disappear with the active codec selection.
    addChildConfigurable(m_data->plugin.get(), "");
    m_data->spec.mode    = mode;
    m_data->spec.quality = params;
  }

  void ImageCompressor::setCompression(const CompressionSpec &spec) {
    installPlugin(spec.mode, spec.quality);
    if (prop("mode").value != spec.mode) setPropertyValue("mode", spec.mode);
  }

  ImageCompressor::CompressionSpec ImageCompressor::getCompression() const {
    return m_data->spec;
  }

  ImageCompressor::CompressedData
  ImageCompressor::compress(const Image &img, bool skipMetaData) {
    if (img.isNull()) {
      throw ICLException("ImageCompressor::compress: image is null");
    }

    // Encode payload via the active plugin.
    const CompressionPlugin::Bytes payload = m_data->plugin->compress(img);

    // Assemble envelope.
    EnvelopeFields f;
    f.params      = img.ptr()->getParams();
    f.d           = img.getDepth();
    f.timestamp   = img.getTime();
    f.codecName   = m_data->plugin->name();
    f.codecParams = m_data->plugin->getCodecParamsString();
    f.meta        = skipMetaData ? std::string{} : img.ptr()->getMetaData();

    const int envSz   = envelopeSize(f);
    const int totalSz = envSz + static_cast<int>(payload.len);

    m_data->envelopeBuf.resize(static_cast<std::size_t>(totalSz));
    writeEnvelope(m_data->envelopeBuf.data(), f);
    std::memcpy(m_data->envelopeBuf.data() + envSz, payload.data, payload.len);

    // Compute compression ratio over the codec payload only (envelope
    // overhead is fixed and small; reporting raw vs. payload is the
    // useful metric for "how well did the codec do").
    const std::size_t rawLen = static_cast<std::size_t>(img.getChannels())
                             * img.getDim() * getSizeOf(img.getDepth());
    const float ratio = rawLen ? static_cast<float>(payload.len) / static_cast<float>(rawLen) : 1.f;

    return CompressedData(m_data->envelopeBuf.data(), totalSz, ratio, m_data->spec);
  }

  Image ImageCompressor::uncompress(const icl8u *bytes, int len) {
    EnvelopeFields f;
    const int payloadOffset = parseEnvelope(bytes, len, f);

    // Cache the decode plugin if the codec name didn't change between calls.
    if (m_data->decodePluginName != f.codecName) {
      m_data->decodePlugin     = compressionRegistry().getOrThrow(f.codecName).payload();
      m_data->decodePluginName = f.codecName;
    }
    m_data->decodePlugin->setCodecParamsString(f.codecParams);

    CompressionPlugin::Bytes payload{
      bytes + payloadOffset,
      static_cast<std::size_t>(len - payloadOffset)
    };
    Image out = m_data->decodePlugin->decompress(payload, f.params, f.d);

    // Restore meta data + timestamp (the plugin only sees the codec
    // payload — these are envelope-level fields).
    if (!f.meta.empty()) out.ptr()->getMetaData() = f.meta;
    out.ptr()->setTime(f.timestamp);

    m_data->decoded = out;
    return out;
  }

  Time ImageCompressor::pickTimeStamp(const icl8u *bytes, int len) {
    EnvelopeFields f;
    parseEnvelope(bytes, len, f);
    return f.timestamp;
  }
} // namespace icl::io
