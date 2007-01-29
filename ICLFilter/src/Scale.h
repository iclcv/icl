#ifndef SCALE_H
#define SCALE_H

#include "Affine.h"

namespace icl{
  
  /// Class to scale images
  class Scale : private Affine{
    public:
    /// Constructor
    Scale (double factorX=0.0, double factorY=0.0, scalemode eInterpolate=interpolateLIN) :
      Affine (eInterpolate) {}
    
    /// change rotation angle
    void setTranslation (double factorX, double factorY) {
      Affine::reset (); 
      Affine::scale (factorX,factorY);
    }
    /// apply should still be public
    Affine::apply;
  };
}
#endif
