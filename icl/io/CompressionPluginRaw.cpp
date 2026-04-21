// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// `raw` codec: planar concatenation of channel bytes, no transform.
// Lossless, supports any depth and channel count. Always built.

#include <icl/io/CompressionPlugin.h>
#include <icl/io/CompressionRegister.h>
#include <icl/core/CoreFunctions.h>
#include <icl/core/ImgBase.h>
#include <cstring>
#include <vector>

namespace icl::io {
  using namespace icl::core;
  using namespace icl::utils;

  namespace {
    class RawPlugin : public CompressionPlugin {
      std::vector<icl8u> m_buf;

    public:
      std::string name() const override { return "raw"; }

      Bytes compress(const Image &src) override {
        const ImgBase *p = src.ptr();
        const int nChan = p->getChannels();
        const int dim   = p->getDim();
        const int bpp   = getSizeOf(p->getDepth());
        m_buf.resize(static_cast<std::size_t>(nChan) * dim * bpp);
        icl8u *dst = m_buf.data();
        for (int c = 0; c < nChan; ++c) {
          std::memcpy(dst, p->getDataPtr(c), dim * bpp);
          dst += dim * bpp;
        }
        return {m_buf.data(), m_buf.size()};
      }

      Image decompress(Bytes bytes, const ImgParams &params,
                       depth d) override {
        Image out(params.getSize(), d, params.getChannels(),
                  params.getFormat());
        out.ptr()->setROI(params.getROI());
        const int dim = out.getDim();
        const int bpp = getSizeOf(d);
        const icl8u *src = bytes.data;
        for (int c = 0; c < out.getChannels(); ++c) {
          std::memcpy(out.ptr()->getDataPtr(c), src, dim * bpp);
          src += dim * bpp;
        }
        return out;
      }
    };
  }

  REGISTER_COMPRESSION_PLUGIN(raw, [](){
    return std::unique_ptr<CompressionPlugin>(new RawPlugin);
  })
} // namespace icl::io
