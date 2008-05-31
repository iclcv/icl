#ifndef ICL_LUT_2D_H
#define ICL_LUT_2D_H

namespace icl{
  
  /// Simple 2D indexed LUT Implementation
  /** The LUT2D template class provides functionalities for
      aranging 2D index data in a linear array for best
      and constant access performance, whereby x and y indices
      are allowed to be in an abitrary integer range [minVal,maxVal].
      
      Internally an array of size RANGE² named "lut" is created.
      Date element Lookup can be calculated using the following 
      formula (\f$range := maxVal - minVal\f$):
      \f[
      LUTIDX_{naiv}(x,y) = x-minVal + range \cdot (y-minVal) 
      \f]
      To enhance access performace, the lut-pointer can be adapted
      as follows:
      \f[
      lut -= (range+1)*minVal;
      \f]
      Now, data element lookup can be calculated much easier:
      \f[
      LUTIDX_{optimized}(x,y) = x + range\cdot y 
      \f]

      
      A simple ()-operator can be used to access lut elements.
      
      */
  template<class RESULT_T=float,class IDX_T=int>
  class LUT2D{
    //    RESULT_T *lutOrig;
    RESULT_T *lut;
    int minVal;
    int range;

    public:
    /// creating a new LUT2D object with given element creation function
    LUT2D(RESULT_T (*generator_func)(IDX_T v1, IDX_T v2),IDX_T minVal, IDX_T maxVal):
    minVal(minVal),range(maxVal-minVal){
      
      lut = (new RESULT_T[range*range])-(range+1)*minVal;
      
      for(int i=minVal;i<maxVal;i++){
        for(int j=minVal;j<maxVal;j++){
          lut[i+range*j] = generator_func(i,j);
        }
      }
    }
    /// destructor
    ~LUT2D(){
      delete [] (lut + (range+1)*minVal);
    }
    /// inline access in constant time of data element at (v1,v2) 
    inline RESULT_T operator()(IDX_T v1, IDX_T v2) const{
      return lut[v1+range*v2];
    }

  };
}

#endif
