/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/UnaryArithmeticalOp.cpp        **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Andre Justus                      **
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

#include <ICLFilter/UnaryArithmeticalOp.h>
#include <ICLCore/Img.h>
#include <ICLUtils/SSETypes.h>
#include <cmath>
#include <ICLCore/Image.h>
#include <ICLCore/Visitors.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter{
    namespace{
      ///-----------------------------------------------------------------------------------------------
      ///          no val
      ///-----------------------------------------------------------------------------------------------
      template<class T> struct AbsFunc{ static inline T f(const T t) { return abs(t); }  };
      template<> struct AbsFunc<icl32f>{ static inline icl32f f(const icl32f t) { return fabs(t); }  };
      template<> struct AbsFunc<icl64f>{ static inline icl64f f(const icl64f t) { return fabs(t); }  };


      template<class T, UnaryArithmeticalOp::optype OT> struct PixelFuncNoVal{
        static inline T apply(const T t){ return t; }
      };
      template<class T> struct PixelFuncNoVal<T,UnaryArithmeticalOp::sqrOp>{
        static inline T apply(const T t){ return t*t; }
      };
      template<class T> struct PixelFuncNoVal<T,UnaryArithmeticalOp::sqrtOp>{
        static inline T apply(const T t){ return clipped_cast<double,T>(sqrt(static_cast<double>(t))); }
      };
      template<class T> struct PixelFuncNoVal<T,UnaryArithmeticalOp::lnOp>{
        static inline T apply(const T t){ return clipped_cast<double,T>(log(static_cast<double>(t))); }
      };
      template<class T> struct PixelFuncNoVal<T,UnaryArithmeticalOp::expOp>{
        static inline T apply(const T t){ return clipped_cast<double,T>(exp(static_cast<double>(t))); }
      };
      template<class T> struct PixelFuncNoVal<T,UnaryArithmeticalOp::absOp>{
        static inline T apply(const T t){ return AbsFunc<T>::f(t); }
      };
      template<> struct PixelFuncNoVal<icl8u,UnaryArithmeticalOp::absOp>{
        static inline icl8u apply(const icl8u t){ return t; }
      };

      template<class T, UnaryArithmeticalOp::optype OT> struct LoopFuncNoVal{
        static inline void apply(const Img<T> *src, Img<T> *dst){
          visitROILinesPerChannelWith(*src, *dst, [](const T *s, T *d, int, int w) {
            for(int i = 0; i < w; ++i) d[i] = PixelFuncNoVal<T, OT>::apply(s[i]);
          });
        }
      };


#ifdef ICL_HAVE_SSE2
      // --- SSE2 specializations for icl32f (no val) ---
      template<> struct LoopFuncNoVal<icl32f, UnaryArithmeticalOp::sqrOp>{
        static inline void apply(const Img<icl32f> *src, Img<icl32f> *dst){
          visitROILinesPerChannelWith(*src, *dst, [](const icl32f *s, icl32f *d, int, int w) {
            int i = 0;
            for(; i <= w-4; i += 4){
              __m128 v = _mm_loadu_ps(s+i);
              _mm_storeu_ps(d+i, _mm_mul_ps(v, v));
            }
            for(; i < w; ++i) d[i] = s[i] * s[i];
          });
        }
      };
      template<> struct LoopFuncNoVal<icl32f, UnaryArithmeticalOp::absOp>{
        static inline void apply(const Img<icl32f> *src, Img<icl32f> *dst){
          __m128 signMask = _mm_set1_ps(-0.0f);
          visitROILinesPerChannelWith(*src, *dst, [signMask](const icl32f *s, icl32f *d, int, int w) {
            int i = 0;
            for(; i <= w-4; i += 4){
              _mm_storeu_ps(d+i, _mm_andnot_ps(signMask, _mm_loadu_ps(s+i)));
            }
            for(; i < w; ++i) d[i] = fabs(s[i]);
          });
        }
      };
      template<> struct LoopFuncNoVal<icl32f, UnaryArithmeticalOp::sqrtOp>{
        static inline void apply(const Img<icl32f> *src, Img<icl32f> *dst){
          visitROILinesPerChannelWith(*src, *dst, [](const icl32f *s, icl32f *d, int, int w) {
            int i = 0;
            for(; i <= w-4; i += 4){
              _mm_storeu_ps(d+i, _mm_sqrt_ps(_mm_loadu_ps(s+i)));
            }
            for(; i < w; ++i) d[i] = sqrtf(s[i]);
          });
        }
      };
#endif

      ///-----------------------------------------------------------------------------------------------
      ///          with val
      ///-----------------------------------------------------------------------------------------------
      template<class T, UnaryArithmeticalOp::optype OT> struct PixelFuncWithVal{
        static inline T apply(const T t, T val){ (void)t; (void)val; return t; }
      };
      template<class T> struct PixelFuncWithVal<T,UnaryArithmeticalOp::addOp>{
        static inline T apply(const T t, T val){ return clipped_cast<icl64f,T>(static_cast<icl64f>(t)+static_cast<icl64f>(val)); }
      };
      template<class T> struct PixelFuncWithVal<T,UnaryArithmeticalOp::subOp>{
        static inline T apply(const T t, T val){ return clipped_cast<icl64f,T>(static_cast<icl64f>(t)-static_cast<icl64f>(val)); }
      };
      template<class T> struct PixelFuncWithVal<T,UnaryArithmeticalOp::mulOp>{
        static inline T apply(const T t, T val){ return clipped_cast<icl64f,T>(static_cast<icl64f>(t)*static_cast<icl64f>(val)); }
      };
      template<class T> struct PixelFuncWithVal<T,UnaryArithmeticalOp::divOp>{
        static inline T apply(const T t, T val){ return clipped_cast<icl64f,T>(static_cast<icl64f>(t)/static_cast<icl64f>(val)); }
      };

      template<class T, UnaryArithmeticalOp::optype OT> struct LoopFuncWithVal{
        static inline void apply(const Img<T> *src, Img<T> *dst, T val){
          visitROILinesPerChannelWith(*src, *dst, [val](const T *s, T *d, int, int w) {
            for(int i = 0; i < w; ++i) d[i] = PixelFuncWithVal<T, OT>::apply(s[i], val);
          });
        }
      };

#ifdef ICL_HAVE_SSE2
      // SSE2 specialization macro for icl32f with-val ops
#define UNARY_ARITH_SSE2_WITH_VAL_32F(OPTYPE, SSE_INSTR)                              \
      template<> struct LoopFuncWithVal<icl32f, UnaryArithmeticalOp::OPTYPE>{          \
        static inline void apply(const Img<icl32f> *src, Img<icl32f> *dst, icl32f val){   \
          __m128 vv = _mm_set1_ps(val);                                                \
          visitROILinesPerChannelWith(*src, *dst, [vv, val](const icl32f *s, icl32f *d, int, int w) { \
            int i = 0;                                                                 \
            for(; i <= w-4; i += 4){                                                   \
              _mm_storeu_ps(d+i, SSE_INSTR(_mm_loadu_ps(s+i), vv));                    \
            }                                                                          \
            for(; i < w; ++i) d[i] = PixelFuncWithVal<icl32f, UnaryArithmeticalOp::OPTYPE>::apply(s[i], val); \
          });                                                                          \
        }                                                                              \
      };

      UNARY_ARITH_SSE2_WITH_VAL_32F(addOp, _mm_add_ps)
      UNARY_ARITH_SSE2_WITH_VAL_32F(subOp, _mm_sub_ps)
      UNARY_ARITH_SSE2_WITH_VAL_32F(mulOp, _mm_mul_ps)
      UNARY_ARITH_SSE2_WITH_VAL_32F(divOp, _mm_div_ps)

#undef UNARY_ARITH_SSE2_WITH_VAL_32F
#endif

#ifdef ICL_HAVE_IPP
      // IPP specializations omitted for now (ICL_HAVE_IPP is not set on this platform)
#endif

    } // end of anonymous namespace


    void UnaryArithmeticalOp::apply(const Image &src, Image &dst) {
      if(!prepare(dst, src)) return;
      src.visitWith(dst, [this](const auto &s, auto &d) {
        using T = typename std::remove_reference_t<decltype(s)>::type;
        switch(m_eOpType){
          case addOp:  LoopFuncWithVal<T, addOp>::apply(&s, &d, clipped_cast<icl64f,T>(m_dValue)); break;
          case mulOp:  LoopFuncWithVal<T, mulOp>::apply(&s, &d, clipped_cast<icl64f,T>(m_dValue)); break;
          case divOp:  LoopFuncWithVal<T, divOp>::apply(&s, &d, clipped_cast<icl64f,T>(m_dValue)); break;
          case subOp:  LoopFuncWithVal<T, subOp>::apply(&s, &d, clipped_cast<icl64f,T>(m_dValue)); break;
          case sqrOp:  LoopFuncNoVal<T, sqrOp>::apply(&s, &d); break;
          case sqrtOp: LoopFuncNoVal<T, sqrtOp>::apply(&s, &d); break;
          case lnOp:   LoopFuncNoVal<T, lnOp>::apply(&s, &d); break;
          case expOp:  LoopFuncNoVal<T, expOp>::apply(&s, &d); break;
          case absOp:  LoopFuncNoVal<T, absOp>::apply(&s, &d); break;
        }
      });
    }

  } // namespace filter
}
