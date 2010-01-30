#ifndef ICL_DIST_FROM_PROGARG_UTIL_H
#define ICL_DIST_FROM_PROGARG_UTIL_H


#include <ICLUtils/ProgArg.h>

namespace icl{

  /// utility class for getting distortion parameters from progarg environtment
  /** example usage (using additional cpp-define DIST_FROM_PROGARG:
      \code
      GenerigGrabber g(...)
      if(pa("-dist")){
         g.enableDistortion(DIST_FROM_PROGARG("-dist"),g.getDesiredSize());
      }
      
      
      int main(int n, char **ppc){
        painit(n,ppc,"-dist(float,float,float,float) ...");
      }
      \endcode
  */
  struct DistFromProgArgUtil{
    static double p[4];
    /// Construktor
    inline DistFromProgArgUtil(const std::string &s){
      p[0] = pa(s,0);
      p[1] = pa(s,1);
      p[2] = pa(s,2);
      p[3] = pa(s,3);
    }
    /// converts to double* (passable to Grabber::enableDistortion)
    operator double*(){
      return p;
    }
  };

#define DIST_FROM_PROGARG(S) DistFromProgArgUtil(S)

}

#endif
