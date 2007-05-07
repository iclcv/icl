#ifndef ROTATE_OP_H
#define ROTATE_OP_H

#include "iclAffineOp.h"

namespace icl{
  
 /// Class to rotate images
  class RotateOp : public AffineOp {
    public:
    /// Constructor
    RotateOp (double dAngle=0.0, scalemode eInterpolate=interpolateLIN) :
      AffineOp (eInterpolate) {}
    
    /// sets the rotation angle
    /**
      @param dAngle angle in degrees (clockwise) 
    */
    void setAngle (double dAngle) {
      AffineOp::reset ();
      AffineOp::rotate (dAngle);
    }
    
    // apply should still be public
    ///applies the rotation
    AffineOp::apply;

    private: // hide the following methods
    AffineOp::translate;
    AffineOp::scale;
  };
}
#endif
