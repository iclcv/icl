#include "ippiFunctions.h"
#include <algorithm>

namespace icl {

// {{{ Threshold_LT

template <typename T, IppStatus (*ippiFunc) (const T*, int, T*, int, IppiSize, T)>
inline void Threshold_LT (const Img<T>& src, Img<T>& dst, T tThreshold) {
   for (int c=std::min(src.getChannels(), dst.getChannels()) - 1; 
        c >= 0; --c) {
      ippiFunc (src.getROIData (c), src.getLineStep(),
                dst.getROIData (c), dst.getLineStep(),
                dst.getROISize(), tThreshold);
   }
}
template <>
void Threshold_LT<icl8u> (const Img<icl8u>& src, Img<icl8u>& dst, icl8u tThreshold) {
   Threshold_LT<icl8u, ippiThreshold_LT_8u_C1R> (src, dst, tThreshold);
}
template<>
void Threshold_LT<icl32f> (const Img<icl32f>& src, Img<icl32f>& dst, icl32f tThreshold) {
   Threshold_LT<icl32f, ippiThreshold_LT_32f_C1R> (src, dst, tThreshold);
}

// }}}
// {{{ Threshold_GT

template <typename T, IppStatus (*ippiFunc) (const T*, int, T*, int, IppiSize, T)>
inline void Threshold_GT (const Img<T>& src, Img<T>& dst, T tThreshold) {
   for (int c=std::min(src.getChannels(), dst.getChannels()) - 1; 
        c >= 0; --c) {
      ippiFunc (src.getROIData (c), src.getLineStep(),
                dst.getROIData (c), dst.getLineStep(),
                dst.getROISize(), tThreshold);
   }
}
template<>
void Threshold_GT<icl8u> (const Img<icl8u>& src, Img<icl8u>& dst, icl8u tThreshold) {
   Threshold_GT<icl8u, ippiThreshold_GT_8u_C1R> (src, dst, tThreshold);
}
template<>
void Threshold_GT<icl32f> (const Img<icl32f>& src, Img<icl32f>& dst, icl32f tThreshold) {
   Threshold_GT<icl32f, ippiThreshold_GT_32f_C1R> (src, dst, tThreshold);
}

// }}}
// {{{ Threshold_LTValGTVal

template <typename T, IppStatus (*ippiFunc) (const T*, int, T*, int, IppiSize, T, T, T, T)>
inline void Threshold_LTValGTVal (const Img<T>& src, Img<T>& dst, 
                           T tLowerThreshold, T tLowerValue,
                           T tUpperThreshold, T tUpperValue) {
   for (int c=std::min(src.getChannels(), dst.getChannels()) - 1; 
        c >= 0; --c) {
      ippiFunc (src.getROIData (c), src.getLineStep(),
                dst.getROIData (c), dst.getLineStep(),
                dst.getROISize(), 
                tLowerThreshold, tLowerValue,
                tUpperThreshold, tUpperValue);
   }
}
template<>
void Threshold_LTValGTVal<icl8u> (const Img<icl8u>& src, Img<icl8u>& dst, 
                                  icl8u tLowerThreshold, icl8u tLowerValue,
                                  icl8u tUpperThreshold, icl8u tUpperValue) {
   Threshold_LTValGTVal<icl8u, ippiThreshold_LTValGTVal_8u_C1R> 
      (src, dst, tLowerThreshold, tLowerValue, tUpperThreshold, tUpperValue);
}
template<>
void Threshold_LTValGTVal<icl32f> (const Img<icl32f>& src, Img<icl32f>& dst, 
                                   icl32f tLowerThreshold, icl32f tLowerValue,
                                   icl32f tUpperThreshold, icl32f tUpperValue) {
   Threshold_LTValGTVal<icl32f, ippiThreshold_LTValGTVal_32f_C1R> 
      (src, dst, tLowerThreshold, tLowerValue, tUpperThreshold, tUpperValue);
}

// }}}

// {{{ Compare

template <typename T, IppStatus (*ippiFunc) (const T*, int, const T*, int, 
                                             Ipp8u*, int, IppiSize, IppCmpOp)>
inline void Compare (const Img<T>& src1, const Img<T>& src2, Img<icl8u>& dst, IppCmpOp op) {
   for (int c=std::min(std::min(src1.getChannels(), src2.getChannels()), 
                       dst.getChannels()) - 1; 
        c >= 0; --c) {
      ippiFunc (src1.getROIData (c), src1.getLineStep(),
                src2.getROIData (c), src2.getLineStep(),
                dst.getROIData (c), dst.getLineStep(),
                dst.getROISize (), op);
   }
}
template <>
void Compare<icl8u> (const Img<icl8u>& src1, const Img<icl8u>& src2, 
                     Img<icl8u>& dst, IppCmpOp op) {
   Compare<icl8u, ippiCompare_8u_C1R> (src1, src2, dst, op);
}
template <>
void Compare<icl32f> (const Img<icl32f>& src1, const Img<icl32f>& src2, 
                      Img<icl8u>& dst, IppCmpOp op) {
   Compare<icl32f, ippiCompare_32f_C1R> (src1, src2, dst, op);
}

// }}}
// {{{ CompareC

template <typename T, IppStatus (*ippiFunc) (const T*, int, T value,
                                             Ipp8u*, int, IppiSize, IppCmpOp)>
inline void CompareC (const Img<T>& src, T value, Img<icl8u>& dst, IppCmpOp op) {
   for (int c=std::min(src.getChannels(), dst.getChannels()) - 1; 
        c >= 0; --c) {
      ippiFunc (src.getROIData (c), src.getLineStep(), value, 
                dst.getROIData (c), dst.getLineStep(),
                dst.getROISize (), op);
   }
}
template <>
void CompareC<icl8u> (const Img<icl8u>& src, icl8u value, 
                      Img<icl8u>& dst, IppCmpOp op) {
   CompareC<icl8u, ippiCompareC_8u_C1R> (src, value, dst, op);
}
template <>
void CompareC<icl32f> (const Img<icl32f>& src, icl32f value,
                       Img<icl8u>& dst, IppCmpOp op) {
   CompareC<icl32f, ippiCompareC_32f_C1R> (src, value, dst, op);
}

// }}}
// {{{ CompareEqualEps

void CompareEqualEps (const Img<icl32f>& src1, const Img<icl32f>& src2, 
                      Img<icl8u>& dst, float fEps) {
   for (int c=std::min(std::min(src1.getChannels(), src2.getChannels()), 
                       dst.getChannels()) - 1; 
        c >= 0; --c) {
      ippiCompareEqualEps_32f_C1R (src1.getROIData (c), src1.getLineStep(),
                                   src2.getROIData (c), src2.getLineStep(),
                                   dst.getROIData (c), dst.getLineStep(),
                                   dst.getROISize (), fEps);
   }
}

// }}}
// {{{ CompareEqualEpsC

void CompareEqualEpsC (const Img<icl32f>& src, float fValue, 
                       Img<icl8u>& dst, float fEps) {
   for (int c=std::min(src.getChannels(), dst.getChannels()) - 1; 
        c >= 0; --c) {
      ippiCompareEqualEpsC_32f_C1R (src.getROIData (c), src.getLineStep(), fValue,
                                    dst.getROIData (c), dst.getLineStep(),
                                    dst.getROISize (), fEps);
   }
}

// }}}

}
