/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/UnaryCompareOp_Simd.cpp     **
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
#include <ICLFilter/UnaryCompareOp.h>

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

  // --- 8u compare: mask IS the result (0xFF = 255, 0x00 = 0) ---
  using CmpOp = icl::filter::UnaryCompareOp;

  void simd_compare_8u(const Img8u &src, Img8u &dst, icl8u value, int optype) {
    __m128i vv = _mm_set1_epi8(static_cast<char>(value));
    auto scalar = [&](const icl8u *s, icl8u *d, int w, int start) {
      for(int i = start; i < w; ++i) {
        switch(optype) {
          case CmpOp::lt:   d[i] = s[i] < value ? 255 : 0; break;
          case CmpOp::lteq: d[i] = s[i] <= value ? 255 : 0; break;
          case CmpOp::eq:   d[i] = s[i] == value ? 255 : 0; break;
          case CmpOp::gteq: d[i] = s[i] >= value ? 255 : 0; break;
          case CmpOp::gt:   d[i] = s[i] > value ? 255 : 0; break;
          default: break;
        }
      }
    };

    visitROILinesPerChannelWith(src, dst, [&](const icl8u *s, icl8u *d, int, int w) {
      int i = 0;
      for(; i <= w-16; i += 16) {
        __m128i val = _mm_loadu_si128(reinterpret_cast<const __m128i*>(s+i));
        __m128i mask;
        switch(optype) {
          case CmpOp::lt:   mask = sse_unsigned_cmplt_epu8(val, vv); break;
          case CmpOp::lteq: mask = _mm_or_si128(sse_unsigned_cmplt_epu8(val, vv), _mm_cmpeq_epi8(val, vv)); break;
          case CmpOp::eq:   mask = _mm_cmpeq_epi8(val, vv); break;
          case CmpOp::gteq: mask = _mm_or_si128(sse_unsigned_cmpgt_epu8(val, vv), _mm_cmpeq_epi8(val, vv)); break;
          case CmpOp::gt:   mask = sse_unsigned_cmpgt_epu8(val, vv); break;
          default:          mask = _mm_setzero_si128(); break;
        }
        _mm_storeu_si128(reinterpret_cast<__m128i*>(d+i), mask);
      }
      scalar(s, d, w, i);
    });
  }


  void simd_compare(const Image &src, Image &dst, double value, int optype) {
    simd_compare_8u(src.as8u(), dst.as8u(), static_cast<icl8u>(value), optype);
  }

  // --- Self-registration ---
  static const int _reg1 = registerBackend<CmpOp::CmpSig>(
    "UnaryCompareOp.compare", Backend::Simd, simd_compare,
    applicableTo<icl8u>, "SSE2/NEON compare (8u)");

} // anon namespace

#endif // ICL_HAVE_SSE2
