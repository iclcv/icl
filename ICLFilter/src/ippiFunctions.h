#ifndef ICL_IPP_FUNCTIONS_H
#define ICL_IPP_FUNCTIONS_H

#include <Img.h>

namespace icl {

// Threshold functions
template <typename T>
void Threshold_LT (const Img<T>& src, Img<T>& dst, T tThreshold);

template <typename T>
void Threshold_GT (const Img<T>& src, Img<T>& dst, T tThreshold);

template <typename T>
void Threshold_LTValGTVal (const Img<T>& src, Img<T>& dst, 
                           T tLowerThreshold, T tLowerValue,
                           T tUpperThreshold, T tUpperValue);

// Compare functions
template <typename T>
void Compare (const Img<T>& src1, const Img<T>& src2, 
              Img<icl8u>& dst, IppCmpOp op);

template <typename T>
void CompareC (const Img<T>& src, T value, Img<icl8u>& dst, IppCmpOp op);

void CompareEqualEps (const Img<icl32f>& src1, const Img<icl32f>& src2, 
                      Img<icl8u>& dst, float fEps);

void CompareEqualEpsC (const Img<icl32f>& src1, float fValue, 
                       Img<icl8u>& dst, float fEps);


} // namespace icl

#endif // ICL_IPP_FUNCTIONS_H
