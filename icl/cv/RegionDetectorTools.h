// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Erik Weitnauer

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/BasicTypes.h>
#include <icl/utils/SSETypes.h>

namespace icl::cv {
  namespace region_detector_tools{

    template<class IteratorA, class IteratorB, class Predicate>
    inline void copy_if(IteratorA srcBegin, IteratorA srcEnd, IteratorB dstBegin, Predicate p){
      while(srcBegin != srcEnd){
        if(p(*srcBegin)){
          *dstBegin = *srcBegin;
          ++dstBegin;
        }
        ++srcBegin;
      }
    }

    template<class T>
    inline const T *find_first_not(const T *first,const T* last, T val){
      int n = static_cast<int>((last - first) >> 3);

      for (; n ; --n){
#define REGION_DETECTOR_2_ONE if(*first != val) return first; ++first;
        REGION_DETECTOR_2_ONE
        REGION_DETECTOR_2_ONE
        REGION_DETECTOR_2_ONE
        REGION_DETECTOR_2_ONE
        REGION_DETECTOR_2_ONE
        REGION_DETECTOR_2_ONE
        REGION_DETECTOR_2_ONE
        REGION_DETECTOR_2_ONE
        }
      switch (last - first){
#define REGION_DETECTOR_2_ONE_R(REST) case REST: REGION_DETECTOR_2_ONE
        REGION_DETECTOR_2_ONE_R(7)
        REGION_DETECTOR_2_ONE_R(6)
        REGION_DETECTOR_2_ONE_R(5)
        REGION_DETECTOR_2_ONE_R(4)
        REGION_DETECTOR_2_ONE_R(3)
        REGION_DETECTOR_2_ONE_R(2)
        REGION_DETECTOR_2_ONE_R(1)
        case 0: default: return last;
      }
#undef REGION_DETECTOR_2_ONE
#undef REGION_DETECTOR_2_ONE_R
    }

    template<class T>
    inline const T *find_first_not_no_opt(const T *first,const T* last, T val){
      int n = static_cast<int>((last - first) >> 3);

      for (; n ; --n){
#define REGION_DETECTOR_2_ONE if(*first != val) return first; ++first;
        REGION_DETECTOR_2_ONE REGION_DETECTOR_2_ONE
        REGION_DETECTOR_2_ONE REGION_DETECTOR_2_ONE
        REGION_DETECTOR_2_ONE REGION_DETECTOR_2_ONE
        REGION_DETECTOR_2_ONE REGION_DETECTOR_2_ONE
        }
      switch (last - first){
#define REGION_DETECTOR_2_ONE_R(REST) case REST: REGION_DETECTOR_2_ONE
        REGION_DETECTOR_2_ONE_R(7)
        REGION_DETECTOR_2_ONE_R(6)
        REGION_DETECTOR_2_ONE_R(5)
        REGION_DETECTOR_2_ONE_R(4)
        REGION_DETECTOR_2_ONE_R(3)
        REGION_DETECTOR_2_ONE_R(2)
        REGION_DETECTOR_2_ONE_R(1)
        case 0: default: return last;
      }
#undef REGION_DETECTOR_2_ONE
#undef REGION_DETECTOR_2_ONE_R
    }

#define REGION_DETECTOR_2_USE_OPT_4_BYTES


#ifdef ICL_HAVE_SSE2
    // SSE2: scan 16 bytes at a time using _mm_cmpeq_epi8 + _mm_movemask_epi8
    template<>
    inline const icl8u *find_first_not(const icl8u *first, const icl8u *last, icl8u val){
      // scalar lead-in until 16-byte aligned
      while(first < last && (reinterpret_cast<uintptr_t>(first) & 0xF)){
        if(*first != val) return first;
        ++first;
      }
      if(first >= last) return last;

      __m128i vval = _mm_set1_epi8(static_cast<char>(val));
      while(first + 16 <= last){
        __m128i v = _mm_load_si128(reinterpret_cast<const __m128i*>(first));
        int mask = _mm_movemask_epi8(_mm_cmpeq_epi8(v, vval));
        if(mask != 0xFFFF){
          // at least one byte differs — find which one
          // mask has 1 for equal, 0 for not-equal; invert to find first not-equal
          int idx = __builtin_ctz(~mask);
          return first + idx;
        }
        first += 16;
      }
      // scalar tail
      while(first < last && *first == val) ++first;
      return first;
    }
#elif defined(REGION_DETECTOR_2_USE_OPT_4_BYTES)
    // 4-byte optimization fallback
    template<>
    inline const icl8u *find_first_not(const icl8u *first, const icl8u *last, icl8u val){
      while( first < last && (reinterpret_cast<uintptr_t>(first) & 0x3) ){
        if(*first != val) return first;
        ++first;
      }
      if(first >= last) return first;
      unsigned int n = (last-first)/4;
      const icl32u *p32 = find_first_not(reinterpret_cast<const icl32u*>(first),
                                         reinterpret_cast<const icl32u*>(first)+n,
                                         static_cast<icl32u>(val | (val<<8) | (val<<16) | (val<<24)) );
      const icl8u *p8u = reinterpret_cast<const icl8u*>(p32);
      while(p8u < last && *p8u == val) ++p8u;
      return p8u;
    }
#endif

  }
  } // namespace icl::cv