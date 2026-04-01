// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLCore/ImageBackendDispatching.h>
#include <ICLCore/Image.h>
#include <ICLCore/Visitors.h>
#include <ICLUtils/ClippedCast.h>
#include <ICLFilter/ThresholdOp.h>

#ifdef ICL_HAVE_SSE2
#include <ICLUtils/SSETypes.h>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  // --- Unsigned byte compare helpers ---
  static inline __m128i sse_unsigned_cmplt_epu8(__m128i a, __m128i b){
    __m128i bias = _mm_set1_epi8(static_cast<char>(0x80));
    return _mm_cmplt_epi8(_mm_xor_si128(a, bias), _mm_xor_si128(b, bias));
  }
  static inline __m128i sse_unsigned_cmpgt_epu8(__m128i a, __m128i b){
    __m128i bias = _mm_set1_epi8(static_cast<char>(0x80));
    return _mm_cmpgt_epi8(_mm_xor_si128(a, bias), _mm_xor_si128(b, bias));
  }

  // --- SSE2 threshold implementations ---
  // These work at the Image level: visitWith dispatches on depth,
  // then if constexpr selects the SIMD path for supported types.


  void simd_ltval(const Image &src, Image &dst, double threshold, double value) {
    src.visitWith(dst, [&](const auto &s, auto &d) {
      using T = typename std::remove_reference_t<decltype(s)>::type;
      if constexpr (std::is_same_v<T, icl32f>) {
        icl32f t = static_cast<icl32f>(threshold), v = static_cast<icl32f>(value);
        __m128 vt = _mm_set1_ps(t);
        __m128 vv = _mm_set1_ps(v);
        visitROILinesPerChannelWith(s, d, [vt, vv, t, v](const icl32f *sp, icl32f *dp, int, int w) {
          int i = 0;
          for(; i <= w-4; i += 4){
            __m128 val = _mm_loadu_ps(sp+i);
            __m128 mask = _mm_cmplt_ps(val, vt);
            _mm_storeu_ps(dp+i, _mm_or_ps(_mm_and_ps(mask, vv), _mm_andnot_ps(mask, val)));
          }
          for(; i < w; ++i) dp[i] = (sp[i] < t) ? v : sp[i];
        });
      } else if constexpr (std::is_same_v<T, icl8u>) {
        icl8u t = static_cast<icl8u>(threshold), v = static_cast<icl8u>(value);
        __m128i vt = _mm_set1_epi8(static_cast<char>(t));
        __m128i vv = _mm_set1_epi8(static_cast<char>(v));
        visitROILinesPerChannelWith(s, d, [vt, vv, t, v](const icl8u *sp, icl8u *dp, int, int w) {
          int i = 0;
          for(; i <= w-16; i += 16){
            __m128i val = _mm_loadu_si128(reinterpret_cast<const __m128i*>(sp+i));
            __m128i mask = sse_unsigned_cmplt_epu8(val, vt);
            _mm_storeu_si128(reinterpret_cast<__m128i*>(dp+i),
              _mm_or_si128(_mm_and_si128(mask, vv), _mm_andnot_si128(mask, val)));
          }
          for(; i < w; ++i) dp[i] = (sp[i] < t) ? v : sp[i];
        });
      }
    });
  }

  void simd_gtval(const Image &src, Image &dst, double threshold, double value) {
    src.visitWith(dst, [&](const auto &s, auto &d) {
      using T = typename std::remove_reference_t<decltype(s)>::type;
      if constexpr (std::is_same_v<T, icl32f>) {
        icl32f t = static_cast<icl32f>(threshold), v = static_cast<icl32f>(value);
        __m128 vt = _mm_set1_ps(t);
        __m128 vv = _mm_set1_ps(v);
        visitROILinesPerChannelWith(s, d, [vt, vv, t, v](const icl32f *sp, icl32f *dp, int, int w) {
          int i = 0;
          for(; i <= w-4; i += 4){
            __m128 val = _mm_loadu_ps(sp+i);
            __m128 mask = _mm_cmpgt_ps(val, vt);
            _mm_storeu_ps(dp+i, _mm_or_ps(_mm_and_ps(mask, vv), _mm_andnot_ps(mask, val)));
          }
          for(; i < w; ++i) dp[i] = (sp[i] > t) ? v : sp[i];
        });
      } else if constexpr (std::is_same_v<T, icl8u>) {
        icl8u t = static_cast<icl8u>(threshold), v = static_cast<icl8u>(value);
        __m128i vt = _mm_set1_epi8(static_cast<char>(t));
        __m128i vv = _mm_set1_epi8(static_cast<char>(v));
        visitROILinesPerChannelWith(s, d, [vt, vv, t, v](const icl8u *sp, icl8u *dp, int, int w) {
          int i = 0;
          for(; i <= w-16; i += 16){
            __m128i val = _mm_loadu_si128(reinterpret_cast<const __m128i*>(sp+i));
            __m128i mask = sse_unsigned_cmpgt_epu8(val, vt);
            _mm_storeu_si128(reinterpret_cast<__m128i*>(dp+i),
              _mm_or_si128(_mm_and_si128(mask, vv), _mm_andnot_si128(mask, val)));
          }
          for(; i < w; ++i) dp[i] = (sp[i] > t) ? v : sp[i];
        });
      }
    });
  }

  void simd_ltgtval(const Image &src, Image &dst,
                    double tLo, double vLo, double tHi, double vHi) {
    src.visitWith(dst, [&](const auto &s, auto &d) {
      using T = typename std::remove_reference_t<decltype(s)>::type;
      if constexpr (std::is_same_v<T, icl32f>) {
        icl32f lo = static_cast<icl32f>(tLo), vl = static_cast<icl32f>(vLo);
        icl32f hi = static_cast<icl32f>(tHi), vh = static_cast<icl32f>(vHi);
        __m128 vtLow = _mm_set1_ps(lo), vvLow = _mm_set1_ps(vl);
        __m128 vtUp  = _mm_set1_ps(hi), vvUp  = _mm_set1_ps(vh);
        visitROILinesPerChannelWith(s, d, [=](const icl32f *sp, icl32f *dp, int, int w) {
          int i = 0;
          for(; i <= w-4; i += 4){
            __m128 v = _mm_loadu_ps(sp+i);
            __m128 maskLo = _mm_cmplt_ps(v, vtLow);
            __m128 maskHi = _mm_cmpgt_ps(v, vtUp);
            __m128 result = v;
            result = _mm_or_ps(_mm_and_ps(maskLo, vvLow), _mm_andnot_ps(maskLo, result));
            result = _mm_or_ps(_mm_and_ps(maskHi, vvUp),  _mm_andnot_ps(maskHi, result));
            _mm_storeu_ps(dp+i, result);
          }
          for(; i < w; ++i){
            icl32f v = sp[i];
            dp[i] = (v < lo) ? vl : (v > hi) ? vh : v;
          }
        });
      } else if constexpr (std::is_same_v<T, icl8u>) {
        icl8u lo = static_cast<icl8u>(tLo), vl = static_cast<icl8u>(vLo);
        icl8u hi = static_cast<icl8u>(tHi), vh = static_cast<icl8u>(vHi);
        __m128i vtLow = _mm_set1_epi8(static_cast<char>(lo));
        __m128i vvLow = _mm_set1_epi8(static_cast<char>(vl));
        __m128i vtUp  = _mm_set1_epi8(static_cast<char>(hi));
        __m128i vvUp  = _mm_set1_epi8(static_cast<char>(vh));
        visitROILinesPerChannelWith(s, d, [=](const icl8u *sp, icl8u *dp, int, int w) {
          int i = 0;
          for(; i <= w-16; i += 16){
            __m128i v = _mm_loadu_si128(reinterpret_cast<const __m128i*>(sp+i));
            __m128i maskLo = sse_unsigned_cmplt_epu8(v, vtLow);
            __m128i maskHi = sse_unsigned_cmpgt_epu8(v, vtUp);
            __m128i result = v;
            result = _mm_or_si128(_mm_and_si128(maskLo, vvLow), _mm_andnot_si128(maskLo, result));
            result = _mm_or_si128(_mm_and_si128(maskHi, vvUp),  _mm_andnot_si128(maskHi, result));
            _mm_storeu_si128(reinterpret_cast<__m128i*>(dp+i), result);
          }
          for(; i < w; ++i){
            icl8u v = sp[i];
            dp[i] = (v < lo) ? vl : (v > hi) ? vh : v;
          }
        });
      }
    });
  }

  // --- Direct registration into the class prototype ---
  using TOp = icl::filter::ThresholdOp;
  using Op = TOp::Op;

  static int _reg = [] {
    auto simd = TOp::prototype().backends(Backend::Simd);
    simd.add<TOp::ThreshSig>(Op::ltVal, simd_ltval,
      applicableTo<icl8u, icl32f>, "SSE2 threshold ltVal (8u/32f)");
    simd.add<TOp::ThreshSig>(Op::gtVal, simd_gtval,
      applicableTo<icl8u, icl32f>, "SSE2 threshold gtVal (8u/32f)");
    simd.add<TOp::ThreshDualSig>(Op::ltgtVal, simd_ltgtval,
      applicableTo<icl8u, icl32f>, "SSE2 threshold ltgtVal (8u/32f)");
    return 0;
  }();

} // anon namespace

#endif // ICL_HAVE_SSE2
