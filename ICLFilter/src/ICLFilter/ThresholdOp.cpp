/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/ThresholdOp.cpp                **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Robert Haschke, Andre Justus      **
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

#include <ICLFilter/ThresholdOp.h>
#include <ICLCore/Img.h>
#include <ICLUtils/SSETypes.h>
#include <ICLCore/Image.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter{

    ThresholdOp::ThresholdOp(optype ttype,float lowThreshold, float highThreshold,float lowVal, float highVal){
      m_eType=ttype;
      m_fLowThreshold=lowThreshold;
      m_fHighThreshold=highThreshold;
      m_fLowVal=lowVal;
      m_fHighVal=highVal;
    }
    ThresholdOp::~ThresholdOp(){
    }

    // --- C++ fallback functors ---

    template <typename T> class ThreshOpLTVal {
    public:
       ThreshOpLTVal (T t, T v) : threshold(t), value(v) {}
       inline T operator()(T val) const {
          if (val < threshold) return value;
          return val;
       }
    private:
       T threshold;
       T value;
    };

    template <typename T> class ThreshOpGTVal {
    public:
       ThreshOpGTVal (T t, T v) : threshold(t), value(v) {}
       inline T operator()(T val) const {
          if (val > threshold) return value;
          return val;
       }
    private:
       T threshold;
       T value;
    };

    template <typename T> class ThreshOpLTGTVal {
    public:
       ThreshOpLTGTVal(T tLow, T vLow, T tUp, T vUp) :
          tLow(tLow), tUp(tUp), vLow(vLow), vUp(vUp) {}
       inline T operator()(T val) const {
          if (val < tLow) return vLow;
          if (val > tUp)  return vUp;
          return val;
       }
    private:
       T tLow, tUp;
       T vLow, vUp;
    };

    // --- Generic fallback loop ---

    template <typename T, class ThresholdFunc>
    inline void fallbackThreshold(const Img<T> &src, Img<T> &dst,
                                  const ThresholdFunc &threshold) {
       for(int c=src.getChannels()-1; c >= 0; --c) {
          const ImgIterator<T> itSrc = src.beginROI(c);
          const ImgIterator<T> itSrcEnd = src.endROI(c);
          ImgIterator<T> itDst = dst.beginROI(c);
          for(;itSrc != itSrcEnd; ++itSrc, ++itDst){
             *itDst = threshold(*itSrc);
          }
       }
    }

    // --- SSE2/NEON SIMD helper functions ---

#ifdef ICL_HAVE_SSE2

    inline void sse_threshold_lt_val_32f(const Img<icl32f> &src, Img<icl32f> &dst,
                                         icl32f threshold, icl32f value){
      __m128 vt = _mm_set1_ps(threshold);
      __m128 vv = _mm_set1_ps(value);
      for(int c=src.getChannels()-1; c >= 0; --c){
        const icl32f *s = src.getROIData(c);
        icl32f *d = dst.getROIData(c);
        int n = src.getROISize().getDim();
        int i = 0;
        for(; i <= n-4; i += 4){
          __m128 v = _mm_loadu_ps(s+i);
          __m128 mask = _mm_cmplt_ps(v, vt);
          _mm_storeu_ps(d+i, _mm_or_ps(_mm_and_ps(mask, vv), _mm_andnot_ps(mask, v)));
        }
        for(; i < n; ++i) d[i] = (s[i] < threshold) ? value : s[i];
      }
    }

    inline void sse_threshold_gt_val_32f(const Img<icl32f> &src, Img<icl32f> &dst,
                                         icl32f threshold, icl32f value){
      __m128 vt = _mm_set1_ps(threshold);
      __m128 vv = _mm_set1_ps(value);
      for(int c=src.getChannels()-1; c >= 0; --c){
        const icl32f *s = src.getROIData(c);
        icl32f *d = dst.getROIData(c);
        int n = src.getROISize().getDim();
        int i = 0;
        for(; i <= n-4; i += 4){
          __m128 v = _mm_loadu_ps(s+i);
          __m128 mask = _mm_cmpgt_ps(v, vt);
          _mm_storeu_ps(d+i, _mm_or_ps(_mm_and_ps(mask, vv), _mm_andnot_ps(mask, v)));
        }
        for(; i < n; ++i) d[i] = (s[i] > threshold) ? value : s[i];
      }
    }

    inline void sse_threshold_ltgt_val_32f(const Img<icl32f> &src, Img<icl32f> &dst,
                                           icl32f tLow, icl32f vLow, icl32f tUp, icl32f vUp){
      __m128 vtLow = _mm_set1_ps(tLow);
      __m128 vvLow = _mm_set1_ps(vLow);
      __m128 vtUp  = _mm_set1_ps(tUp);
      __m128 vvUp  = _mm_set1_ps(vUp);
      for(int c=src.getChannels()-1; c >= 0; --c){
        const icl32f *s = src.getROIData(c);
        icl32f *d = dst.getROIData(c);
        int n = src.getROISize().getDim();
        int i = 0;
        for(; i <= n-4; i += 4){
          __m128 v = _mm_loadu_ps(s+i);
          __m128 maskLo = _mm_cmplt_ps(v, vtLow);
          __m128 maskHi = _mm_cmpgt_ps(v, vtUp);
          __m128 result = v;
          result = _mm_or_ps(_mm_and_ps(maskLo, vvLow), _mm_andnot_ps(maskLo, result));
          result = _mm_or_ps(_mm_and_ps(maskHi, vvUp),  _mm_andnot_ps(maskHi, result));
          _mm_storeu_ps(d+i, result);
        }
        for(; i < n; ++i){
          icl32f v = s[i];
          d[i] = (v < tLow) ? vLow : (v > tUp) ? vUp : v;
        }
      }
    }

    // Unsigned byte compare helpers (SSE2 only has signed _mm_cmpgt_epi8).
    // XOR with 0x80 maps [0,255] to [-128,127] for correct unsigned comparison.
    static inline __m128i sse_unsigned_cmplt_epu8(__m128i a, __m128i b){
      __m128i bias = _mm_set1_epi8(static_cast<char>(0x80));
      return _mm_cmplt_epi8(_mm_xor_si128(a, bias), _mm_xor_si128(b, bias));
    }
    static inline __m128i sse_unsigned_cmpgt_epu8(__m128i a, __m128i b){
      __m128i bias = _mm_set1_epi8(static_cast<char>(0x80));
      return _mm_cmpgt_epi8(_mm_xor_si128(a, bias), _mm_xor_si128(b, bias));
    }

    inline void sse_threshold_lt_val_8u(const Img<icl8u> &src, Img<icl8u> &dst,
                                        icl8u threshold, icl8u value){
      __m128i vt = _mm_set1_epi8(static_cast<char>(threshold));
      __m128i vv = _mm_set1_epi8(static_cast<char>(value));
      for(int c=src.getChannels()-1; c >= 0; --c){
        const icl8u *s = src.getROIData(c);
        icl8u *d = dst.getROIData(c);
        int n = src.getROISize().getDim();
        int i = 0;
        for(; i <= n-16; i += 16){
          __m128i v = _mm_loadu_si128(reinterpret_cast<const __m128i*>(s+i));
          __m128i mask = sse_unsigned_cmplt_epu8(v, vt);
          __m128i result = _mm_or_si128(_mm_and_si128(mask, vv), _mm_andnot_si128(mask, v));
          _mm_storeu_si128(reinterpret_cast<__m128i*>(d+i), result);
        }
        for(; i < n; ++i) d[i] = (s[i] < threshold) ? value : s[i];
      }
    }

    inline void sse_threshold_gt_val_8u(const Img<icl8u> &src, Img<icl8u> &dst,
                                        icl8u threshold, icl8u value){
      __m128i vt = _mm_set1_epi8(static_cast<char>(threshold));
      __m128i vv = _mm_set1_epi8(static_cast<char>(value));
      for(int c=src.getChannels()-1; c >= 0; --c){
        const icl8u *s = src.getROIData(c);
        icl8u *d = dst.getROIData(c);
        int n = src.getROISize().getDim();
        int i = 0;
        for(; i <= n-16; i += 16){
          __m128i v = _mm_loadu_si128(reinterpret_cast<const __m128i*>(s+i));
          __m128i mask = sse_unsigned_cmpgt_epu8(v, vt);
          __m128i result = _mm_or_si128(_mm_and_si128(mask, vv), _mm_andnot_si128(mask, v));
          _mm_storeu_si128(reinterpret_cast<__m128i*>(d+i), result);
        }
        for(; i < n; ++i) d[i] = (s[i] > threshold) ? value : s[i];
      }
    }

    inline void sse_threshold_ltgt_val_8u(const Img<icl8u> &src, Img<icl8u> &dst,
                                          icl8u tLow, icl8u vLow, icl8u tUp, icl8u vUp){
      __m128i vtLow = _mm_set1_epi8(static_cast<char>(tLow));
      __m128i vvLow = _mm_set1_epi8(static_cast<char>(vLow));
      __m128i vtUp  = _mm_set1_epi8(static_cast<char>(tUp));
      __m128i vvUp  = _mm_set1_epi8(static_cast<char>(vUp));
      for(int c=src.getChannels()-1; c >= 0; --c){
        const icl8u *s = src.getROIData(c);
        icl8u *d = dst.getROIData(c);
        int n = src.getROISize().getDim();
        int i = 0;
        for(; i <= n-16; i += 16){
          __m128i v = _mm_loadu_si128(reinterpret_cast<const __m128i*>(s+i));
          __m128i maskLo = sse_unsigned_cmplt_epu8(v, vtLow);
          __m128i maskHi = sse_unsigned_cmpgt_epu8(v, vtUp);
          __m128i result = v;
          result = _mm_or_si128(_mm_and_si128(maskLo, vvLow), _mm_andnot_si128(maskLo, result));
          result = _mm_or_si128(_mm_and_si128(maskHi, vvUp),  _mm_andnot_si128(maskHi, result));
          _mm_storeu_si128(reinterpret_cast<__m128i*>(d+i), result);
        }
        for(; i < n; ++i){
          icl8u v = s[i];
          d[i] = (v < tLow) ? vLow : (v > tUp) ? vUp : v;
        }
      }
    }

#endif // ICL_HAVE_SSE2

    // --- IPP helper templates ---

#ifdef ICL_HAVE_IPP
    template <typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize, T, T)>
    inline void ippiThresholdCall_2T(const Img<T> &src, Img<T> &dst, T t1, T t2){
       for (int c=src.getChannels()-1; c >= 0; --c) {
          ippiFunc (src.getROIData (c), src.getLineStep(),
                    dst.getROIData (c), dst.getLineStep(),
                    dst.getROISize(), t1, t2);
       }
    }

    template <typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, T*, int, IppiSize, T, T, T, T)>
    inline void ippiThresholdCall_4T(const Img<T> &src, Img<T> &dst, T t1,T t2, T t3, T t4){
       for (int c=src.getChannels()-1; c >= 0; --c) {
          ippiFunc (src.getROIData (c), src.getLineStep(),
                    dst.getROIData (c), dst.getLineStep(),
                    dst.getROISize(), t1,t2,t3,t4);
       }
    }
#endif // ICL_HAVE_IPP

    // =====================================================
    //  Dispatch structs: primary template = C++ fallback,
    //  specializations override for IPP or SSE2/NEON.
    // =====================================================

    // --- ThreshLTVal ---

    template<class T> struct ThreshLTVal {
      static void apply(const Img<T> &src, Img<T> &dst, T t, T val) {
        fallbackThreshold(src, dst, ThreshOpLTVal<T>(t, val));
      }
    };
#if defined(ICL_HAVE_IPP)
    template<> struct ThreshLTVal<icl8u> {
      static void apply(const Img8u &src, Img8u &dst, icl8u t, icl8u val) {
        ippiThresholdCall_2T<icl8u, ippiThreshold_LTVal_8u_C1R>(src, dst, t, val);
      }
    };
    template<> struct ThreshLTVal<icl16s> {
      static void apply(const Img16s &src, Img16s &dst, icl16s t, icl16s val) {
        ippiThresholdCall_2T<icl16s, ippiThreshold_LTVal_16s_C1R>(src, dst, t, val);
      }
    };
    template<> struct ThreshLTVal<icl32f> {
      static void apply(const Img32f &src, Img32f &dst, icl32f t, icl32f val) {
        ippiThresholdCall_2T<icl32f, ippiThreshold_LTVal_32f_C1R>(src, dst, t, val);
      }
    };
#elif defined(ICL_HAVE_SSE2)
    template<> struct ThreshLTVal<icl8u> {
      static void apply(const Img8u &src, Img8u &dst, icl8u t, icl8u val) {
        sse_threshold_lt_val_8u(src, dst, t, val);
      }
    };
    template<> struct ThreshLTVal<icl32f> {
      static void apply(const Img32f &src, Img32f &dst, icl32f t, icl32f val) {
        sse_threshold_lt_val_32f(src, dst, t, val);
      }
    };
#endif

    // --- ThreshGTVal ---

    template<class T> struct ThreshGTVal {
      static void apply(const Img<T> &src, Img<T> &dst, T t, T val) {
        fallbackThreshold(src, dst, ThreshOpGTVal<T>(t, val));
      }
    };
#if defined(ICL_HAVE_IPP)
    template<> struct ThreshGTVal<icl8u> {
      static void apply(const Img8u &src, Img8u &dst, icl8u t, icl8u val) {
        ippiThresholdCall_2T<icl8u, ippiThreshold_GTVal_8u_C1R>(src, dst, t, val);
      }
    };
    template<> struct ThreshGTVal<icl16s> {
      static void apply(const Img16s &src, Img16s &dst, icl16s t, icl16s val) {
        ippiThresholdCall_2T<icl16s, ippiThreshold_GTVal_16s_C1R>(src, dst, t, val);
      }
    };
    template<> struct ThreshGTVal<icl32f> {
      static void apply(const Img32f &src, Img32f &dst, icl32f t, icl32f val) {
        ippiThresholdCall_2T<icl32f, ippiThreshold_GTVal_32f_C1R>(src, dst, t, val);
      }
    };
#elif defined(ICL_HAVE_SSE2)
    template<> struct ThreshGTVal<icl8u> {
      static void apply(const Img8u &src, Img8u &dst, icl8u t, icl8u val) {
        sse_threshold_gt_val_8u(src, dst, t, val);
      }
    };
    template<> struct ThreshGTVal<icl32f> {
      static void apply(const Img32f &src, Img32f &dst, icl32f t, icl32f val) {
        sse_threshold_gt_val_32f(src, dst, t, val);
      }
    };
#endif

    // --- ThreshLTGTVal ---

    template<class T> struct ThreshLTGTVal {
      static void apply(const Img<T> &src, Img<T> &dst, T tLo, T vLo, T tHi, T vHi) {
        fallbackThreshold(src, dst, ThreshOpLTGTVal<T>(tLo, vLo, tHi, vHi));
      }
    };
#if defined(ICL_HAVE_IPP)
    template<> struct ThreshLTGTVal<icl8u> {
      static void apply(const Img8u &src, Img8u &dst, icl8u tLo, icl8u vLo, icl8u tHi, icl8u vHi) {
        ippiThresholdCall_4T<icl8u, ippiThreshold_LTValGTVal_8u_C1R>(src, dst, tLo, vLo, tHi, vHi);
      }
    };
    template<> struct ThreshLTGTVal<icl16s> {
      static void apply(const Img16s &src, Img16s &dst, icl16s tLo, icl16s vLo, icl16s tHi, icl16s vHi) {
        ippiThresholdCall_4T<icl16s, ippiThreshold_LTValGTVal_16s_C1R>(src, dst, tLo, vLo, tHi, vHi);
      }
    };
    template<> struct ThreshLTGTVal<icl32f> {
      static void apply(const Img32f &src, Img32f &dst, icl32f tLo, icl32f vLo, icl32f tHi, icl32f vHi) {
        ippiThresholdCall_4T<icl32f, ippiThreshold_LTValGTVal_32f_C1R>(src, dst, tLo, vLo, tHi, vHi);
      }
    };
#elif defined(ICL_HAVE_SSE2)
    template<> struct ThreshLTGTVal<icl8u> {
      static void apply(const Img8u &src, Img8u &dst, icl8u tLo, icl8u vLo, icl8u tHi, icl8u vHi) {
        sse_threshold_ltgt_val_8u(src, dst, tLo, vLo, tHi, vHi);
      }
    };
    template<> struct ThreshLTGTVal<icl32f> {
      static void apply(const Img32f &src, Img32f &dst, icl32f tLo, icl32f vLo, icl32f tHi, icl32f vHi) {
        sse_threshold_ltgt_val_32f(src, dst, tLo, vLo, tHi, vHi);
      }
    };
#endif

    // =====================================================

    void ThresholdOp::apply(const core::Image &src, core::Image &dst) {
      if(!prepare(dst, src)) return;
      src.visitWith(dst, [this](const auto &s, auto &d) {
        using T = typename std::remove_reference_t<decltype(s)>::type;
        T tLo = clipped_cast<icl32f,T>(m_fLowThreshold);
        T tHi = clipped_cast<icl32f,T>(m_fHighThreshold);
        T vLo = clipped_cast<icl32f,T>(m_fLowVal);
        T vHi = clipped_cast<icl32f,T>(m_fHighVal);
        switch(m_eType){
          case lt:      ThreshLTVal<T>::apply(s, d, tLo, tLo); break;
          case gt:      ThreshGTVal<T>::apply(s, d, tHi, tHi); break;
          case ltgt:    ThreshLTGTVal<T>::apply(s, d, tLo, tLo, tHi, tHi); break;
          case ltVal:   ThreshLTVal<T>::apply(s, d, tLo, vLo); break;
          case gtVal:   ThreshGTVal<T>::apply(s, d, tHi, vHi); break;
          case ltgtVal: ThreshLTGTVal<T>::apply(s, d, tLo, vLo, tHi, vHi); break;
        }
      });
    }

  } // namespace filter
} // namespace icl
