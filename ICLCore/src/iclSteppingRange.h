#ifndef STEPPING_RANGE_H
#define STEPPING_RANGE_H

#include "iclRange.h"

namespace icl{
  /// class representing a range with defined stepping \ingroup TYPES
  /** A Stepping range is a well defined interval
      I = [minVal,maxVal] with an also well defined 
      stepping e.g. S. the SteppingRange contains all
      values {minVal, minVal+S, minVal+2*S,...} that
      are less then or equal to the maxVal 
  **/
  template<class Type> 
  struct SteppingRange : public Range<Type>{
    /// create an empty range (min = max = 0)
    SteppingRange():Range<Type>(),stepping(0){}
    
    /// create a special Rage
    SteppingRange(Type minVal, Type maxVal, Type stepping): 
      Range<Type>(minVal,maxVal), stepping(stepping){}

    /// internal stepping parameter
    Type stepping;
    
    /// casts this range into another depth
    template<class dstType>
    const SteppingRange<dstType> castTo() const{
      return SteppingRange<dstType>(clipped_cast<Type,dstType>(Range<Type>::minVal),
                                    clipped_cast<Type,dstType>(Range<Type>::maxVal),
                                    clipped_cast<Type,dstType>(stepping));
    }
    
    /// tests whether a given value is inside of this range
    virtual bool contains(Type value) const { 
      if(stepping == 0) return Range<Type>(Range<Type>::minVal,Range<Type>::maxVal).contains(value);
      Type offs = value - Range<Type>::minVal;
      int n = (int)(offs/stepping);
      double n2 = (double)offs/stepping;
      return Range<Type>::contains(value) && double(n) == n2;
    }
    
    /// returns the nearest value to the given one 
    /** -1st: value is clipped to the underlying range
        -2nd: if value is inside the range, the nearest
              implicit step is returned
    **/
    Type nearest(Type value){
      if(value < Range<Type>::minVal) return Range<Type>::minVal;
      if(value > Range<Type>::maxVal) return Range<Type>::maxVal;
      if(stepping == 0) return value;
      Type offs = value - Range<Type>::minVal;
      int n = (int)(offs/stepping);
      Type k = n*stepping;
      if(value-k >= (double)(stepping)/2){
        return k+stepping;
      }else{
        return k;
      }
    }    
  };

  
}
#endif
