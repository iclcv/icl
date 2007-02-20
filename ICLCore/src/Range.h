#ifndef RANGE_H
#define RANGE_H

#include "ICLCore.h"
namespace icl{
  /// class representing a range defined by min and max value
  template<class Type> 
  struct Range{
    /// create an empty range (min = max = 0)
    Range():minVal(Type(0)),maxVal(Type(0)){}
    
    /// create a special Rage
    Range(Type minVal, Type maxVal): minVal(minVal), maxVal(maxVal){}

    /// minimum value of this range
    Type minVal;
    
    /// maximum value of this range
    Type maxVal;
    
    /// return max-min
    Type getLength() const { return maxVal - minVal; } 
    
    template<class dstType>
    const Range<dstType> castTo() const{
      return Range<dstType>(Cast<Type,dstType>::cast(minVal),Cast<Type,dstType>::cast(maxVal));
    }
  };
}
#endif
