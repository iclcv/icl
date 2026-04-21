// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// `rlen` codec: per-channel run-length encoding for icl8u images.
// Quality 1/4/6/8 maps to bits-per-value (the rest is run-length); see
// ImageCompressor.h for the historical description. Always built.

#include <icl/io/CompressionPlugin.h>
#include <icl/io/CompressionRegister.h>
#include <icl/core/CoreFunctions.h>
#include <icl/core/Img.h>
#include <icl/utils/StringUtils.h>
#include <icl/utils/Exception.h>
#include <vector>

namespace icl::io {
  using namespace icl::core;
  using namespace icl::utils;

  namespace {
    static const icl8u *find_first_not_binarized(const icl8u *curr,
                                                 const icl8u *end,
                                                 icl8u val) {
      if (val) for (; curr < end; ++curr) { if (*curr <  127) return curr; }
      else     for (; curr < end; ++curr) { if (*curr >= 127) return curr; }
      return end;
    }

    // Encode one channel of `imageData` (length `dim` bytes) at `quality`
    // bits-per-value. Returns the new write head into `out`.
    static icl8u *encodeChannel(const icl8u *imageData, int dim,
                                icl8u *out, int quality) {
      const icl8u *imageDataEnd = imageData + dim;
      switch (quality) {
        case 1: {
          icl8u currVal = !!*imageData;
          while (imageData < imageDataEnd) {
            const icl8u *other = find_first_not_binarized(imageData, imageDataEnd, currVal);
            std::size_t len = static_cast<std::size_t>(other - imageData);
            while (len >= 128) { *out++ = 0xff >> int(!currVal); len -= 128; }
            if (len)            *out++ = (len - 1) | (currVal << 7);
            currVal = !currVal;
            imageData = other;
          }
          break;
        }
        case 4: {
          int currVal = *imageData & 0xf0, currLen = 0;
          while (imageData < imageDataEnd) {
            while (imageData < imageDataEnd && (*imageData & 0xf0) == currVal) {
              ++currLen; ++imageData;
            }
            while (currLen >= 16) { *out++ = currVal | 0xf; currLen -= 16; }
            if (currLen)            *out++ = currVal | (currLen - 1);
            currVal = *imageData & 0xf0; currLen = 0;
          }
          break;
        }
        case 6: {
          static const int VAL_MASK = 0xFC, LEN_MASK = 0x3, MAX_LEN = 4;
          int currVal = *imageData & VAL_MASK, currLen = 0;
          while (imageData < imageDataEnd) {
            while (imageData < imageDataEnd && (*imageData & VAL_MASK) == currVal) {
              ++currLen; ++imageData;
            }
            while (currLen >= MAX_LEN) { *out++ = currVal | LEN_MASK; currLen -= MAX_LEN; }
            if (currLen)                *out++ = currVal | (currLen - 1);
            currVal = *imageData & VAL_MASK; currLen = 0;
          }
          break;
        }
        case 8: {
          static const int MAX_LEN = 256;
          int currVal = *imageData, currLen = 0;
          while (imageData < imageDataEnd) {
            while (imageData < imageDataEnd && *imageData == currVal) {
              ++currLen; ++imageData;
            }
            while (currLen >= MAX_LEN) {
              *out++ = currVal; *out++ = MAX_LEN - 1; currLen -= MAX_LEN;
            }
            if (currLen) {
              *out++ = currVal; *out++ = currLen - 1;
            }
            currVal = *imageData; currLen = 0;
          }
          break;
        }
        default:
          throw ICLException("rlen: unsupported quality (" + str(quality) + "); allowed: 1, 4, 6, 8");
      }
      return out;
    }

    // Decode one channel; advances `src` past its consumed bytes.
    static const icl8u *decodeChannel(icl8u *imageData, int dim,
                                      const icl8u *src, int quality) {
      icl8u *pc = imageData, *pcEnd = imageData + dim;
      switch (quality) {
        case 1:
          for (; pc < pcEnd; ++src) {
            const int l = ((*src) & 127) + 1;
            std::fill(pc, pc + l, ((*src) >> 7) * 255);
            pc += l;
          }
          break;
        case 4:
          for (; pc < pcEnd; ++src) {
            const int l = ((*src) & 0xf) + 1;
            std::fill(pc, pc + l, ((*src) & 0xf0));
            pc += l;
          }
          break;
        case 6: {
          static const int VAL_MASK = 0xFC, LEN_MASK = 0x3;
          for (; pc < pcEnd; ++src) {
            const int l = ((*src) & LEN_MASK) + 1;
            std::fill(pc, pc + l, ((*src) & VAL_MASK));
            pc += l;
          }
          break;
        }
        case 8:
          for (; pc < pcEnd; src += 2) {
            const int l = src[1] + 1;
            std::fill(pc, pc + l, src[0]);
            pc += l;
          }
          break;
        default:
          throw ICLException("rlen: unsupported quality (" + str(quality) + ")");
      }
      return src;
    }

    class RlenPlugin : public CompressionPlugin {
      std::vector<icl8u> m_buf;
      int m_quality = 1;

    public:
      RlenPlugin() {
        addProperty("quality", "menu", "1,4,6,8", "1", 0,
                    "Bits-per-value for the RLE token. 1 = binary (best for "
                    "low-noise binary masks); 4/6 = lossy quantization; "
                    "8 = lossless byte-level RLE.");
        Configurable::registerCallback([this](const Property &p){
          if (p.name == "quality") m_quality = parse<int>(p.value);
        });
      }

      std::string name() const override { return "rlen"; }

      Bytes compress(const Image &src) override {
        const ImgBase *p = src.ptr();
        if (p->getDepth() != depth8u) {
          throw ICLException("rlen: only icl8u images are supported");
        }
        const int dim   = p->getDim();
        const int nChan = p->getChannels();
        // Worst-case: q=8 emits 2 bytes per pixel; everyone else <= 1.
        m_buf.resize(static_cast<std::size_t>(nChan) * dim * (m_quality == 8 ? 2 : 1));
        icl8u *out = m_buf.data();
        for (int c = 0; c < nChan; ++c) {
          out = encodeChannel(p->as8u()->getData(c), dim, out, m_quality);
        }
        m_buf.resize(static_cast<std::size_t>(out - m_buf.data()));
        return {m_buf.data(), m_buf.size()};
      }

      Image decompress(Bytes bytes, const ImgParams &params, depth d) override {
        if (d != depth8u) {
          throw ICLException("rlen: only icl8u images are supported");
        }
        Image out(params.getSize(), depth8u, params.getChannels(),
                  params.getFormat());
        out.ptr()->setROI(params.getROI());
        const int dim = out.getDim();
        const icl8u *src = bytes.data;
        for (int c = 0; c < out.getChannels(); ++c) {
          src = decodeChannel(out.ptr()->as8u()->getData(c), dim, src, m_quality);
        }
        return out;
      }

      std::string getCodecParamsString() const override { return str(m_quality); }
      void setCodecParamsString(const std::string &p) override {
        if (!p.empty()) m_quality = parse<int>(p);
      }
    };
  }

  REGISTER_COMPRESSION_PLUGIN(rlen, [](){
    return std::unique_ptr<CompressionPlugin>(new RlenPlugin);
  })
} // namespace icl::io
