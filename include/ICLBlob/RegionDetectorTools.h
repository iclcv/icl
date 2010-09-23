/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLBlob/RegionDetectorTools.h                  **
** Module : ICLBlob                                                **
** Authors: Christof Elbrechter, Erik Weitnauer                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/


#ifndef ICL_REGION_DETECTOR_TOOLS_H
#define ICL_REGION_DETECTOR_TOOLS_H

#include <ICLUtils/BasicTypes.h>

namespace icl{
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
      int n = (int)((last - first) >> 3);
      
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
      int n = (int)((last - first) >> 3);
      
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
    
    
#ifdef REGION_DETECTOR_2_USE_OPT_4_BYTES
    
    template<>
    inline const icl8u *find_first_not(const icl8u *first, const icl8u *last, icl8u val){
#ifdef ICL_32BIT
      while( first < last && (int)first & 0x3 ){
#else
      while( first < last && (int64_t)first & 0x3 ){
#endif
        if(*first != val){
          return first;
        }
        ++first;
      }
      if(first >= last) return first;
      
      unsigned int n = (last-first)/4;
      const icl32u *p32 = find_first_not(reinterpret_cast<const icl32u*>(first),
                                         reinterpret_cast<const icl32u*>(first)+n,
                                         (icl32u)(val | (val<<8) | (val<<16) | (val<<24)) );
      const icl8u *p8u = reinterpret_cast<const icl8u*>(p32);
      while(p8u < last && *p8u == val) ++p8u;
      return p8u;
      //  return find_first_not_no_opt(reinterpret_cast<const icl8u*>(p32),last,val);
    }
#endif

  }
}


#endif  
