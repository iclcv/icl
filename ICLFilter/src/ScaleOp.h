#ifndef SCALE_OP_H
#define SCALE_OP_H

#include "AffineOp.h"

namespace icl{
  
  /// Class to scale images
  class ScaleOp : private AffineOp{
    public:
    /// Constructor
    ScaleOp (double factorX=0.0, double factorY=0.0, scalemode eInterpolate=interpolateLIN) :
      AffineOp (eInterpolate) {}
    
    /// change rotation angle
    void setTranslation (double factorX, double factorY) {
      AffineOp::reset (); 
      AffineOp::scale (factorX,factorY);
    }
    /// apply should still be public
    AffineOp::apply;
  };
}
#endif
