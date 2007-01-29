#ifndef ROTATE_H
#define ROTATE_H

#include "Affine.h"

namespace icl{
  
 /// Class to rotate images
  class Rotate : private Affine {
    public:
    /// Constructor
    Rotate (double dAngle=0.0, scalemode eInterpolate=interpolateLIN) :
      Affine (eInterpolate) {}
    
    /// change rotation angle
    void setAngle (double dAngle) {
      Affine::reset (); Affine::rotate (dAngle);
    }
    
    /// apply should still be public
    Affine::apply;
  };
}
#endif
