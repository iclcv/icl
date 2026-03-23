/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/RegionDetectorTools.h                  **
** Module : ICLCV                                                  **
** Authors: Christof Elbrechter, Erik Weitnauer                    **
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


#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/BasicTypes.h>
#include <ICLUtils/SSETypes.h>

namespace icl{
  namespace cv{
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
  } // namespace cv
}
