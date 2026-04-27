// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// `rlen` codec: per-channel run-length encoding for icl8u images.
//
// `quality` selects the number of value-bits per RLE token; the
// remainder of the token byte holds (run_len - 1) so a single run of
// up to (1 << len_bits) like-valued pixels packs into one byte.  q=8
// is the only lossless variant — it spends a full byte on the value
// and a separate byte on the length.  Lower qualities quantise the
// byte to fewer bits before run-finding (q=1 binarises around the 127
// threshold; q=4/q=6 mask off the low nibble / two bits respectively),
// trading reconstruction fidelity for shorter tokens.
//
// Implementation: a single `RlenCodec<VAL_BITS>` template parameterises
// the four cases.  The encode/decode loops are written once and
// instantiated for VAL_BITS ∈ {1, 4, 6, 8}; per-quality differences
// (mask vs threshold quantisation, single- vs two-byte tokens, value
// expansion on decode) live inside the codec's tiny `quantize`,
// `emit`, and `decodeOne` helpers — no OOB reads, no run-tail
// re-seeds.  Always built (no external dependency).

#include <icl/io/detail/compression-plugins/CompressionPlugin.h>
#include <icl/utils/prop/Constraints.h>
#include <icl/io/detail/compression-plugins/CompressionRegistry.h>
#include <icl/core/CoreFunctions.h>
#include <icl/core/Img.h>
#include <icl/utils/StringUtils.h>
#include <icl/utils/Exception.h>
#include <algorithm>
#include <cstdint>
#include <vector>

namespace icl::io {
  using namespace icl::core;
  using namespace icl::utils;

  namespace {
    // ---- Codec policy -----------------------------------------------
    //
    // VAL_BITS = bits spent on the quantised value in each token.
    // For q=1/4/6 the token is a single byte split into (value | length-1);
    // for q=8 the token is two bytes (value, length-1).  Run-length is
    // the remaining capacity, so MAX_LEN = 1 << LEN_BITS.

    template <int VAL_BITS>
    struct RlenCodec {
      static_assert(VAL_BITS == 1 || VAL_BITS == 4
                 || VAL_BITS == 6 || VAL_BITS == 8,
                    "rlen: only q=1/4/6/8 are supported");

      static constexpr bool TWO_BYTE_TOKEN = (VAL_BITS == 8);
      static constexpr int  LEN_BITS = TWO_BYTE_TOKEN ? 8 : (8 - VAL_BITS);
      static constexpr int  MAX_LEN  = 1 << LEN_BITS;
      static constexpr int  LEN_MASK = MAX_LEN - 1;
      static constexpr int  VAL_MASK = TWO_BYTE_TOKEN
                                       ? 0xff
                                       : (0xff & ~LEN_MASK);
      static constexpr int  TOKEN_BYTES = TWO_BYTE_TOKEN ? 2 : 1;

      // Map a pixel to the run-comparison key.  q=4/6/8 are mask-based
      // (low bits dropped); q=1 is a threshold (>=127) producing the
      // same MSB-set vs MSB-clear pattern as the encoded token, so the
      // emit/decode helpers don't need to special-case binarisation.
      static int quantize(icl8u px) {
        if constexpr (VAL_BITS == 1) {
          return (px >= 127) ? VAL_MASK : 0;  // VAL_MASK == 0x80 here
        } else {
          return px & VAL_MASK;
        }
      }

      // Append a token for a run of `len` (1..MAX_LEN) pixels with
      // quantised value `v` to `out`.  Returns the new write head.
      static icl8u *emit(icl8u *out, int v, int len) {
        if constexpr (TWO_BYTE_TOKEN) {
          *out++ = static_cast<icl8u>(v);
          *out++ = static_cast<icl8u>(len - 1);
        } else {
          *out++ = static_cast<icl8u>(v | (len - 1));
        }
        return out;
      }

      // Decode one token from `src`, expanding it into `dst` (advanced
      // by the run length).  Returns the new read head.  q=1's
      // single-bit value is expanded back to {0, 255} for display
      // fidelity.
      static const icl8u *decodeOne(icl8u *&dst, const icl8u *src) {
        if constexpr (TWO_BYTE_TOKEN) {
          const icl8u v   = src[0];
          const int   len = src[1] + 1;
          std::fill(dst, dst + len, v);
          dst += len;
          return src + 2;
        } else {
          const int t   = *src;
          const int len = (t & LEN_MASK) + 1;
          icl8u v;
          if constexpr (VAL_BITS == 1) {
            v = (t & VAL_MASK) ? 255 : 0;
          } else {
            v = static_cast<icl8u>(t & VAL_MASK);
          }
          std::fill(dst, dst + len, v);
          dst += len;
          return src + 1;
        }
      }
    };

    // ---- Encode / decode loops --------------------------------------
    //
    // A single read of `src[0]` per outer iteration, gated by the
    // `src < end` bounds check at the top of the loop — no OOB on tail
    // (the previous monolithic switch re-seeded `currVal = *src` at
    // the END of each iteration, which read one byte past `end`).

    template <int VAL_BITS>
    static icl8u *encodeChannelT(const icl8u *src, int dim, icl8u *out) {
      using C = RlenCodec<VAL_BITS>;
      const icl8u *end = src + dim;
      while (src < end) {
        const int v = C::quantize(*src);
        int len = 0;
        while (src < end && C::quantize(*src) == v) {
          ++len; ++src;
        }
        while (len >= C::MAX_LEN) {
          out = C::emit(out, v, C::MAX_LEN);
          len -= C::MAX_LEN;
        }
        if (len) {
          out = C::emit(out, v, len);
        }
      }
      return out;
    }

    template <int VAL_BITS>
    static const icl8u *decodeChannelT(icl8u *dst, int dim, const icl8u *src) {
      using C = RlenCodec<VAL_BITS>;
      icl8u *const end = dst + dim;
      while (dst < end) {
        src = C::decodeOne(dst, src);
      }
      return src;
    }

    // Quality-keyed dispatch.  Each branch instantiates the template
    // for one VAL_BITS value; the compiler resolves the inner switch
    // away at template-instantiation time.
    static icl8u *encodeChannel(const icl8u *src, int dim,
                                icl8u *out, int q) {
      switch (q) {
        case 1: return encodeChannelT<1>(src, dim, out);
        case 4: return encodeChannelT<4>(src, dim, out);
        case 6: return encodeChannelT<6>(src, dim, out);
        case 8: return encodeChannelT<8>(src, dim, out);
      }
      throw ICLException("rlen: unsupported quality (" + str(q)
                         + "); allowed: 1, 4, 6, 8");
    }

    static const icl8u *decodeChannel(icl8u *dst, int dim,
                                      const icl8u *src, int q) {
      switch (q) {
        case 1: return decodeChannelT<1>(dst, dim, src);
        case 4: return decodeChannelT<4>(dst, dim, src);
        case 6: return decodeChannelT<6>(dst, dim, src);
        case 8: return decodeChannelT<8>(dst, dim, src);
      }
      throw ICLException("rlen: unsupported quality (" + str(q) + ")");
    }

    // Worst-case bytes-per-pixel for each quality (drives the encoder's
    // pre-allocation).  q=1/4/6 emit a single token byte per maximally-
    // fragmented pixel; q=8 emits two (value + length).
    constexpr int worstBytesPerPixel(int q) {
      return q == 8 ? 2 : 1;
    }

    // ---- Plugin -----------------------------------------------------

    class RlenPlugin : public CompressionPlugin {
      std::vector<icl8u> m_buf;
      int m_quality = 1;

    public:
      RlenPlugin() {
        addProperty("quality", prop::Menu{"1", "4", "6", "8"}, "1",
                    "Bits-per-value for the RLE token. 1 = binary "
                    "(best for low-noise binary masks); 4/6 = lossy "
                    "quantization; 8 = lossless byte-level RLE.");
        Configurable::registerCallback([this](const Property &p){
          if (p.name == "quality") m_quality = p.as<int>();
        });
      }

      std::string name() const override { return "rlen"; }

      Capabilities capabilities() const override {
        return { .depths={depth8u} };
      }

      Bytes compress(const Image &src) override {
        const ImgBase *p = src.ptr();
        if (p->getDepth() != depth8u) {
          throw ICLException("rlen: only icl8u images are supported");
        }
        // Snapshot m_quality once.  The Configurable callback that
        // writes m_quality runs on whichever thread mutated the
        // property (typically the GUI thread via qt::Prop) while
        // compress() runs on a worker thread; reading the field twice
        // would let a 6→8 flip slip past the buffer-size pick and
        // overflow the heap.
        const int q     = m_quality;
        const int dim   = p->getDim();
        const int nChan = p->getChannels();
        m_buf.resize(static_cast<std::size_t>(nChan) * dim
                     * worstBytesPerPixel(q));
        icl8u *out = m_buf.data();
        for (int c = 0; c < nChan; ++c) {
          out = encodeChannel(p->as8u()->getData(c), dim, out, q);
        }
        m_buf.resize(static_cast<std::size_t>(out - m_buf.data()));
        return {m_buf.data(), m_buf.size()};
      }

      Image decompress(Bytes bytes, const ImgParams &params,
                       depth d) override {
        if (d != depth8u) {
          throw ICLException("rlen: only icl8u images are supported");
        }
        // Snapshot m_quality for the same reason as compress() — the
        // wire envelope's setCodecParamsString writes this field on
        // the cached decode plugin.
        const int q = m_quality;
        Image out(params.getSize(), depth8u, params.getChannels(),
                  params.getFormat());
        out.ptr()->setROI(params.getROI());
        const int dim = out.getDim();
        const icl8u *src = bytes.data;
        for (int c = 0; c < out.getChannels(); ++c) {
          src = decodeChannel(out.ptr()->as8u()->getData(c), dim, src, q);
        }
        return out;
      }

      std::string getCodecParamsString() const override {
        return str(m_quality);
      }
      void setCodecParamsString(const std::string &p) override {
        if (!p.empty()) m_quality = parse<int>(p);
      }
    };
  }

  REGISTER_COMPRESSION_PLUGIN(rlen, [](){
    return std::unique_ptr<CompressionPlugin>(new RlenPlugin);
  })
} // namespace icl::io
