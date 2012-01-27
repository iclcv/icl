#ifndef ICL_LINEAR_TRANSFORM_1D_H
#define ICL_LINEAR_TRANSFORM_1D_H

#include <ICLUtils/Range.h>

namespace icl{

  /// A standard linear mapping class for the 1D case f(x) = m * x + b
  struct LinearTransform1D{
    /// slope
    float m;
    
    /// offset
    float b;
    
    /// base constructor with given parameters m and b
    inline LinearTransform1D(float m=0, float b=0):m(m),b(b){}
    
    /// spedical constructor template with given source and destination range
    template<class T>
    inline LinearTransform1D(const Range<T> &s, const Range<T> &d):
      m(d.getLength()/s.getLength()), b(-m*s.minVal + d.minVal){
    }
    
    /// applies the mapping to a given x -> f(x) = m * x + b
    float operator()(float x) const { return m*x+b; }
  };

}

#endif
