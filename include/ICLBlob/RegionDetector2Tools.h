#ifndef ICL_REGION_DETECTOR2_TOOLS_H
#define ICL_REGION_DETECTOR2_TOOLS_H

#include <ICLUtils/BasicTypes.h>

namespace icl{
  namespace region_detector_2_tools{

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
      while( first < last && (int)first & 0x3 ){
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
