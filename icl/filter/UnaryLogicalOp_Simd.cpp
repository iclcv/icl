#include <icl/core/ImageBackendDispatching.h>
#include <icl/core/Image.h>
#include <icl/core/Visitors.h>
#include <icl/filter/UnaryLogicalOp.h>

#ifdef ICL_HAVE_SSE2
#include <icl/utils/SSETypes.h>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  // --- SSE2/NEON bitwise operations ---
  // Process 16 bytes (icl8u) or 4 ints (icl32s) per iteration.
  // icl16s uses the same 128-bit ops as icl32s (bitwise is type-agnostic).

  void simd_withval(const Image &src, Image &dst, icl32s val, int optype) {
    src.visitWith(dst, [val, optype](const auto &s, auto &d) {
      using T = typename std::remove_reference_t<decltype(s)>::type;
      if constexpr (std::is_same_v<T, icl8u>) {
        icl8u v = clipped_cast<icl32s, icl8u>(val);
        __m128i vv = _mm_set1_epi8(static_cast<char>(v));
        visitROILinesPerChannelWith(s, d, [vv, v, optype](const icl8u *sp, icl8u *dp, int, int w) {
          int i = 0;
          for(; i <= w-16; i += 16) {
            __m128i a = _mm_loadu_si128(reinterpret_cast<const __m128i*>(sp+i));
            __m128i r;
            switch(optype) {
              case filter::UnaryLogicalOp::andOp: r = _mm_and_si128(a, vv); break;
              case filter::UnaryLogicalOp::orOp:  r = _mm_or_si128(a, vv); break;
              case filter::UnaryLogicalOp::xorOp: r = _mm_xor_si128(a, vv); break;
              default: r = a; break;
            }
            _mm_storeu_si128(reinterpret_cast<__m128i*>(dp+i), r);
          }
          for(; i < w; ++i) {
            switch(optype) {
              case filter::UnaryLogicalOp::andOp: dp[i] = sp[i] & v; break;
              case filter::UnaryLogicalOp::orOp:  dp[i] = sp[i] | v; break;
              case filter::UnaryLogicalOp::xorOp: dp[i] = sp[i] ^ v; break;
              default: dp[i] = sp[i]; break;
            }
          }
        });
      } else if constexpr (std::is_same_v<T, icl16s> || std::is_same_v<T, icl32s>) {
        T v = clipped_cast<icl32s, T>(val);
        // For 16s and 32s, pack the value into 128-bit register
        __m128i vv;
        if constexpr (std::is_same_v<T, icl16s>)
          vv = _mm_set1_epi16(static_cast<short>(v));
        else
          vv = _mm_set1_epi32(v);

        constexpr int step = 16 / sizeof(T);
        visitROILinesPerChannelWith(s, d, [vv, v, optype](const T *sp, T *dp, int, int w) {
          int i = 0;
          for(; i <= w-step; i += step) {
            __m128i a = _mm_loadu_si128(reinterpret_cast<const __m128i*>(sp+i));
            __m128i r;
            switch(optype) {
              case filter::UnaryLogicalOp::andOp: r = _mm_and_si128(a, vv); break;
              case filter::UnaryLogicalOp::orOp:  r = _mm_or_si128(a, vv); break;
              case filter::UnaryLogicalOp::xorOp: r = _mm_xor_si128(a, vv); break;
              default: r = a; break;
            }
            _mm_storeu_si128(reinterpret_cast<__m128i*>(dp+i), r);
          }
          for(; i < w; ++i) {
            switch(optype) {
              case filter::UnaryLogicalOp::andOp: dp[i] = sp[i] & v; break;
              case filter::UnaryLogicalOp::orOp:  dp[i] = sp[i] | v; break;
              case filter::UnaryLogicalOp::xorOp: dp[i] = sp[i] ^ v; break;
              default: dp[i] = sp[i]; break;
            }
          }
        });
      }
    });
  }

  void simd_noval(const Image &src, Image &dst) {
    src.visitWith(dst, [](const auto &s, auto &d) {
      using T = typename std::remove_reference_t<decltype(s)>::type;
      if constexpr (std::is_integral_v<T>) {
        constexpr int step = 16 / sizeof(T);
        __m128i ones = _mm_set1_epi32(-1);  // all-ones mask for NOT
        visitROILinesPerChannelWith(s, d, [ones](const T *sp, T *dp, int, int w) {
          int i = 0;
          for(; i <= w-step; i += step) {
            __m128i a = _mm_loadu_si128(reinterpret_cast<const __m128i*>(sp+i));
            _mm_storeu_si128(reinterpret_cast<__m128i*>(dp+i), _mm_xor_si128(a, ones));
          }
          for(; i < w; ++i) dp[i] = ~sp[i];
        });
      }
    });
  }

  // --- Direct registration into prototype ---
  using ULOp = icl::filter::UnaryLogicalOp;
  using Op = ULOp::Op;

  static int _reg = [] {
    auto simd = ULOp::prototype().backends(Backend::Simd);
    simd.add<ULOp::WithValSig>(Op::withVal, simd_withval, applicableTo<icl8u, icl16s, icl32s>, "SSE2/NEON logical and/or/xor (8u/16s/32s)");
    simd.add<ULOp::NoValSig>(Op::noVal, simd_noval, applicableTo<icl8u, icl16s, icl32s>, "SSE2/NEON logical not (8u/16s/32s)");
    return 0;
  }();

} // anonymous namespace

#endif // ICL_HAVE_SSE2
