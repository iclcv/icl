#ifndef ICL_BASIC_TYPES_H
#define ICL_BASIC_TYPES_H

#ifdef HAVE_IPP
#include <ipp.h>
#endif

#include <stdint.h>


namespace icl {
  
#ifdef HAVE_IPP
  /// 64Bit floating point type for the ICL \ingroup TYPES
  typedef Ipp64f icl64f;

  /// 32Bit floating point type for the ICL \ingroup TYPES 
  typedef Ipp32f icl32f;

  /// 32bit signed integer type for the ICL \ingroup TYPES
  typedef Ipp32s icl32s;

  /// 16bit signed integer type for the ICL (range [-32767, 32768 ]) \ingroup TYPES
  typedef Ipp16s icl16s;
  
  /// 8Bit unsigned integer type for the ICL \ingroup TYPES
  typedef Ipp8u icl8u;

#else
  /// 64Bit floating point type for the ICL \ingroup TYPES
  typedef double icl64f;

  /// 32Bit floating point type for the ICL \ingroup TYPES
  typedef float icl32f;

  /// 32bit signed integer type for the ICL \ingroup TYPES
  typedef int32_t icl32s;
  
  /// 16bit signed integer type for the ICL (range [-32767, 32768 ]) \ingroup TYPES
  typedef int16_t icl16s;

  /// 8Bit unsigned integer type for the ICL \ingroup TYPES
  typedef uint8_t icl8u;

#endif

  /// 32bit unsigned integer type for the ICL \ingroup TYPES
  typedef uint32_t icl32u;

  /// 16bit unsigned integer type for the ICL \ingroup TYPES
  typedef uint16_t icl16u;
}

#endif // include guard
