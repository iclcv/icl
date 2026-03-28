/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/UnaryArithmeticalOp_Simd.cpp**
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLCore/BackendDispatch.h>
#include <ICLCore/Image.h>
#include <ICLCore/Visitors.h>
#include <ICLFilter/UnaryArithmeticalOp.h>
#include <ICLUtils/ClippedCast.h>
#include <cmath>

#ifdef ICL_HAVE_SSE2
#include <ICLUtils/SSETypes.h>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  using Op = icl::filter::UnaryArithmeticalOp;

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
          case Op::addOp: r = _mm_add_ps(v, vv); break;
          case Op::subOp: r = _mm_sub_ps(v, vv); break;
          case Op::mulOp: r = _mm_mul_ps(v, vv); break;
          case Op::divOp: r = _mm_div_ps(v, vv); break;
          default: r = v; break;
        }
        _mm_storeu_ps(dp+i, r);
      }
      for(; i < w; ++i) {
        switch(optype) {
          case Op::addOp: dp[i] = sp[i] + val; break;
          case Op::subOp: dp[i] = sp[i] - val; break;
          case Op::mulOp: dp[i] = sp[i] * val; break;
          case Op::divOp: dp[i] = sp[i] / val; break;
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
    bool hasSimd = (optype == Op::sqrOp || optype == Op::absOp || optype == Op::sqrtOp);

    visitROILinesPerChannelWith(s, d, [&](const icl32f *sp, icl32f *dp, int, int w) {
      int i = 0;
      if(hasSimd) {
        for(; i <= w-4; i += 4) {
          __m128 v = _mm_loadu_ps(sp+i);
          __m128 r;
          switch(optype) {
            case Op::sqrOp:  r = _mm_mul_ps(v, v); break;
            case Op::absOp:  r = _mm_andnot_ps(signMask, v); break;
            case Op::sqrtOp: r = _mm_sqrt_ps(v); break;
            default: r = v; break;
          }
          _mm_storeu_ps(dp+i, r);
        }
      }
      for(; i < w; ++i) {
        switch(optype) {
          case Op::sqrOp:  dp[i] = sp[i] * sp[i]; break;
          case Op::absOp:  dp[i] = std::fabs(sp[i]); break;
          case Op::sqrtOp: dp[i] = clipped_cast<double,icl32f>(std::sqrt(static_cast<double>(sp[i]))); break;
          case Op::lnOp:   dp[i] = clipped_cast<double,icl32f>(std::log(static_cast<double>(sp[i]))); break;
          case Op::expOp:  dp[i] = clipped_cast<double,icl32f>(std::exp(static_cast<double>(sp[i]))); break;
          default: break;
        }
      }
    });
  }


  // --- Self-registration ---
  static const int _reg1 = registerBackend<Op::ArithValSig>(
    "UnaryArithmeticalOp.withVal", Backend::Simd, simd_arith_with_val,
    applicableTo<icl32f>, "SSE2/NEON arithmetic with-val (32f)");

  static const int _reg2 = registerBackend<Op::ArithNoValSig>(
    "UnaryArithmeticalOp.noVal", Backend::Simd, simd_arith_no_val,
    applicableTo<icl32f>, "SSE2/NEON arithmetic no-val (32f)");

} // anon namespace

#endif // ICL_HAVE_SSE2
