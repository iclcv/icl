#ifndef ICL_DIST_FROM_PROGARG_UTIL_H
#define ICL_DIST_FROM_PROGARG_UTIL_H


#include <iclProgArg.h>

namespace icl{

  /// utility class for getting distortion parameters from progarg environtment
  /** example usage (using additional cpp-define DIST_FROM_PROGARG:
      \code
      GenerigGrabber g(...)
      if(pa_defined("-dist")){
         g.enableDistortion(DIST_FROM_PROGARG("-dist"),g.getDesiredSize());
      }
      
      
      int main(int n, char **ppc){
        pa_init(n,ppc,"-dist(4) ...");
      }
      \endcode
  */
  struct DistFromProgArgUtil{
    static double p[4];
    /// Construktor
    inline DistFromProgArgUtil(const std::string &s){
      p[0] = pa_subarg<double>(s,0,0);
      p[1] = pa_subarg<double>(s,1,0);
      p[2] = pa_subarg<double>(s,2,0);
      p[3] = pa_subarg<double>(s,3,0);
    }
    /// converts to double* (passable to Grabber::enableDistortion)
    operator double*(){
      return p;
    }
  };

#define DIST_FROM_PROGARG(S) DistFromProgArgUtil(S)

}

#endif
