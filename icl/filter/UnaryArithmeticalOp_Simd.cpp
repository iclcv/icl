// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/core/ImageBackendDispatching.h>
#include <icl/core/Image.h>
#include <icl/core/Visitors.h>
#include <icl/filter/UnaryArithmeticalOp.h>
#include <icl/utils/ClippedCast.h>
#include <cmath>

#ifdef ICL_HAVE_SSE2
#include <icl/utils/SSETypes.h>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  using UAOp = icl::filter::UnaryArithmeticalOp;

  // ---- with-val: add/sub/mul/div for icl32f ----
  void simd_arith_with_val(const Image &src, Image &dst, double value, int optype) {
    const Img32f &s = src.as32f();
    Img32f &d = dst.as32f();
    icl32f val = static_cast<icl32f>(value);
    __m128 vv = _mm_set1_ps(val);

    visitROILinesPerChannelWith(s, d, [&](const icl32f *sp, icl32f *dp, int, int w) {
      int i = 0;
      for(; i <= w-4; i += 4) {
        __m128 v = _mm_loadu_ps(sp+i);
        __m128 r;
        switch(optype) {
          case UAOp::addOp: r = _mm_add_ps(v, vv); break;
          case UAOp::subOp: r = _mm_sub_ps(v, vv); break;
          case UAOp::mulOp: r = _mm_mul_ps(v, vv); break;
          case UAOp::divOp: r = _mm_div_ps(v, vv); break;
          default: r = v; break;
        }
        _mm_storeu_ps(dp+i, r);
      }
      for(; i < w; ++i) {
        switch(optype) {
          case UAOp::addOp: dp[i] = sp[i] + val; break;
          case UAOp::subOp: dp[i] = sp[i] - val; break;
          case UAOp::mulOp: dp[i] = sp[i] * val; break;
          case UAOp::divOp: dp[i] = sp[i] / val; break;
          default: break;
        }
      }
    });
  }

  // ---- no-val: sqr/abs/sqrt for icl32f (ln/exp have no SIMD path) ----
  void simd_arith_no_val(const Image &src, Image &dst, int optype) {
    const Img32f &s = src.as32f();
    Img32f &d = dst.as32f();
    __m128 signMask = _mm_set1_ps(-0.0f);
    bool hasSimd = (optype == UAOp::sqrOp || optype == UAOp::absOp || optype == UAOp::sqrtOp);

    visitROILinesPerChannelWith(s, d, [&](const icl32f *sp, icl32f *dp, int, int w) {
      int i = 0;
      if(hasSimd) {
        for(; i <= w-4; i += 4) {
          __m128 v = _mm_loadu_ps(sp+i);
          __m128 r;
          switch(optype) {
            case UAOp::sqrOp:  r = _mm_mul_ps(v, v); break;
            case UAOp::absOp:  r = _mm_andnot_ps(signMask, v); break;
            case UAOp::sqrtOp: r = _mm_sqrt_ps(v); break;
            default: r = v; break;
          }
          _mm_storeu_ps(dp+i, r);
        }
      }
      for(; i < w; ++i) {
        switch(optype) {
          case UAOp::sqrOp:  dp[i] = sp[i] * sp[i]; break;
          case UAOp::absOp:  dp[i] = std::fabs(sp[i]); break;
          case UAOp::sqrtOp: dp[i] = clipped_cast<double,icl32f>(std::sqrt(static_cast<double>(sp[i]))); break;
          case UAOp::lnOp:   dp[i] = clipped_cast<double,icl32f>(std::log(static_cast<double>(sp[i]))); break;
          case UAOp::expOp:  dp[i] = clipped_cast<double,icl32f>(std::exp(static_cast<double>(sp[i]))); break;
          default: break;
        }
      }
    });
  }


  // --- Direct registration into prototype ---
  using Op = UAOp::Op;

  static int _reg = [] {
    auto simd = UAOp::prototype().backends(Backend::Simd);
    simd.add<UAOp::ArithValSig>(Op::withVal, simd_arith_with_val,
      applicableTo<icl32f>, "SSE2/NEON arithmetic with-val (32f)");
    simd.add<UAOp::ArithNoValSig>(Op::noVal, simd_arith_no_val,
      applicableTo<icl32f>, "SSE2/NEON arithmetic no-val (32f)");
    return 0;
  }();

} // anon namespace

#endif // ICL_HAVE_SSE2
