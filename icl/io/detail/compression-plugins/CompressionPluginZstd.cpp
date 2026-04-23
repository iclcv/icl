// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// `zstd` codec: lossless general-purpose compression via libzstd.
// Built only when ICL_HAVE_ZSTD. Treats image bytes as an opaque
// planar concatenation (same layout as the `raw` plugin) — zstd does
// the entropy coding on top.

#ifdef ICL_HAVE_ZSTD

#include <icl/io/detail/compression-plugins/CompressionPlugin.h>
#include <icl/utils/prop/Constraints.h>
#include <icl/io/detail/compression-plugins/CompressionRegistry.h>
#include <icl/core/CoreFunctions.h>
#include <icl/core/Img.h>
#include <icl/core/ImgBase.h>
#include <icl/utils/StringUtils.h>
#include <icl/utils/Exception.h>

#include <zstd.h>
#include <cstring>
#include <vector>

namespace icl::io {
  using namespace icl::core;
  using namespace icl::utils;

  namespace {
    // Pack channels planarly into `dst` (length must be ≥ nChan*dim*bpp).
    static void packPlanar(const ImgBase *p, icl8u *dst) {
      const int nChan = p->getChannels();
      const int dim   = p->getDim();
      const int bpp   = getSizeOf(p->getDepth());
      for (int c = 0; c < nChan; ++c) {
        std::memcpy(dst, p->getDataPtr(c), dim * bpp);
        dst += dim * bpp;
      }
    }
    static void unpackPlanar(const icl8u *src, ImgBase *out) {
      const int nChan = out->getChannels();
      const int dim   = out->getDim();
      const int bpp   = getSizeOf(out->getDepth());
      for (int c = 0; c < nChan; ++c) {
        std::memcpy(out->getDataPtr(c), src, dim * bpp);
        src += dim * bpp;
      }
    }

    class ZstdPlugin : public CompressionPlugin {
      std::vector<icl8u> m_rawBuf;     // planar staging buffer
      std::vector<icl8u> m_compressed; // zstd output
      int m_level = 3;                 // zstd default

    public:
      ZstdPlugin() {
        addProperty("level", prop::Range{.min=1, .max=22, .step=1}, 3, 0,
                    "zstd compression level (1=fastest, 22=smallest). "
                    "Default 3 matches libzstd's ZSTD_CLEVEL_DEFAULT.");
        Configurable::registerCallback([this](const Property &p){
          if (p.name == "level") m_level = p.as<int>();
        });
      }

      std::string name() const override { return "zstd"; }

      Bytes compress(const Image &src) override {
        const ImgBase *p = src.ptr();
        const std::size_t rawLen = static_cast<std::size_t>(p->getChannels())
                                 * p->getDim() * getSizeOf(p->getDepth());
        m_rawBuf.resize(rawLen);
        packPlanar(p, m_rawBuf.data());

        const std::size_t maxCompressed = ZSTD_compressBound(rawLen);
        m_compressed.resize(maxCompressed);
        const std::size_t actual = ZSTD_compress(
          m_compressed.data(), maxCompressed,
          m_rawBuf.data(), rawLen, m_level);
        if (ZSTD_isError(actual)) {
          throw ICLException(std::string("zstd: compress failed: ")
                             + ZSTD_getErrorName(actual));
        }
        m_compressed.resize(actual);
        return {m_compressed.data(), m_compressed.size()};
      }

      Image decompress(Bytes bytes, const ImgParams &params,
                       depth d) override {
        const std::size_t rawLen = static_cast<std::size_t>(params.getChannels())
                                 * params.getSize().getDim() * getSizeOf(d);
        m_rawBuf.resize(rawLen);
        const std::size_t actual = ZSTD_decompress(
          m_rawBuf.data(), rawLen, bytes.data, bytes.len);
        if (ZSTD_isError(actual)) {
          throw ICLException(std::string("zstd: decompress failed: ")
                             + ZSTD_getErrorName(actual));
        }
        if (actual != rawLen) {
          throw ICLException("zstd: decoded size mismatch — image params "
                             "in envelope don't match compressed payload");
        }
        Image out(params.getSize(), d, params.getChannels(), params.getFormat());
        out.ptr()->setROI(params.getROI());
        unpackPlanar(m_rawBuf.data(), out.ptr());
        return out;
      }

      std::string getCodecParamsString() const override { return str(m_level); }
      void setCodecParamsString(const std::string &p) override {
        if (!p.empty()) m_level = parse<int>(p);
      }
    };
  }

  REGISTER_COMPRESSION_PLUGIN(zstd, [](){
    return std::unique_ptr<CompressionPlugin>(new ZstdPlugin);
  })
} // namespace icl::io

#endif // ICL_HAVE_ZSTD
