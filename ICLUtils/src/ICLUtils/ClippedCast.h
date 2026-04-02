// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <algorithm>
#include <limits>

namespace icl::utils {
    /// clips a value into the range [tMin,tMax] \ingroup GENERAL
    template <class T>
    inline T clip(T tX, T tMin, T tMax){
      return std::clamp(tX, tMin, tMax);
    }


    template<class T>
    inline bool is_float_type(){
      return false;
    }

    /** \cond */
    template<> inline bool is_float_type<float>() { return true; }
    template<> inline bool is_float_type<double>() { return true; }
    /** \endcond */

    /// utility cast function wrapping the standard lib's numerical_limits template
    template<class S, class D>
    inline D clipped_cast(S src){
      if(is_float_type<D>()){ //hopefully this is const enough for optimize this expresseion out
        return src < static_cast<S>(-(std::numeric_limits<D>::max)()) ? static_cast<D>(-(std::numeric_limits<D>::max)()) :
        src > static_cast<S>((std::numeric_limits<D>::max)()) ? (std::numeric_limits<D>::max)() :
        static_cast<D>(src);
      }else{
        return src < static_cast<S>((std::numeric_limits<D>::min)()) ? (std::numeric_limits<D>::min)() :
        src > static_cast<S>((std::numeric_limits<D>::max)()) ? (std::numeric_limits<D>::max)() :
        static_cast<D>(src);
      }
    }

    /** \cond */
    /// specializations for all buildin data types
#define SPECIALISE_CLIPPED_CAST(T) template<> inline T clipped_cast<T,T>(T t) { return t; }
    SPECIALISE_CLIPPED_CAST(int)
    SPECIALISE_CLIPPED_CAST(unsigned int)
    SPECIALISE_CLIPPED_CAST(char)
    SPECIALISE_CLIPPED_CAST(unsigned char)
    SPECIALISE_CLIPPED_CAST(short)
    SPECIALISE_CLIPPED_CAST(unsigned short)
    SPECIALISE_CLIPPED_CAST(long)
    SPECIALISE_CLIPPED_CAST(unsigned long)
    SPECIALISE_CLIPPED_CAST(bool)
    SPECIALISE_CLIPPED_CAST(float)
    SPECIALISE_CLIPPED_CAST(double)
  #undef SPECIALISE_CLIPPED_CAST
    /** \endcond */

  } // namespace icl::utils