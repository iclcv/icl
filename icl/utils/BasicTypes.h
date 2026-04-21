// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <stdint.h>
#include <complex>

#ifdef ICL_HAVE_IPP
#include <ipp.h>

#if IPP_VERSION_MAJOR == 6
#define ICL_HAVE_IPP_6X
#endif

#endif

namespace icl {

  // here, we do not use an extra utils namespace

#ifdef ICL_HAVE_IPP
  /// 64Bit floating point type for the ICL \ingroup BASIC_TYPES
  typedef Ipp64f icl64f;

  /// 32Bit floating point type for the ICL \ingroup BASIC_TYPES
  typedef Ipp32f icl32f;

  /// 32bit signed integer type for the ICL \ingroup BASIC_TYPES
  typedef Ipp32s icl32s;

  /// 16bit signed integer type for the ICL (range [-32767, 32768 ]) \ingroup BASIC_TYPES
  typedef Ipp16s icl16s;

  /// 8Bit unsigned integer type for the ICL \ingroup BASIC_TYPES
  typedef Ipp8u icl8u;

#else
  /// 64Bit floating point type for the ICL \ingroup BASIC_TYPES
  typedef double icl64f;

  /// 32Bit floating point type for the ICL \ingroup BASIC_TYPES
  typedef float icl32f;

  /// 32bit signed integer type for the ICL \ingroup BASIC_TYPES
  typedef int32_t icl32s;

  /// 16bit signed integer type for the ICL (range [-32767, 32768 ]) \ingroup BASIC_TYPES
  typedef int16_t icl16s;

  /// 8Bit unsigned integer type for the ICL \ingroup BASIC_TYPES
  typedef uint8_t icl8u;

#endif

  /// 8bit signed integer
  typedef int8_t icl8s;

  /// 32bit unsigned integer type for the ICL \ingroup BASIC_TYPES
  typedef uint32_t icl32u;

  /// 16bit unsigned integer type for the ICL \ingroup BASIC_TYPES
  typedef uint16_t icl16u;

  /// 64bit signed integer type for the ICL \ingroup BASIC_TYPES
  typedef int64_t icl64s;

  /// 64bit unsigned integer type for the ICL \ingroup BASIC_TYPES
  typedef uint64_t icl64u;

  /// float comples type \ingroup BASIC_TYPES
  typedef std::complex<icl32f> icl32c;

  /// float comples type \ingroup BASIC_TYPES
  typedef std::complex<icl64f> icl64c;

}
