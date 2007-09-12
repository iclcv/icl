#ifndef SCALE_OP_H
#define SCALE_OP_H

#include "iclAffineOp.h"

namespace icl{
  
  /// Class to scale images \ingroup UNARY \ingroup AFFINE
  class ScaleOp : public AffineOp{
    public:
    /// Constructor
    ScaleOp (double factorX=0.0, double factorY=0.0, scalemode eInterpolate=interpolateLIN) :
      AffineOp (eInterpolate) {}
    
    /// performs a scale
    /**
      @param factorX scale-factor in x-direction
      @param factorY scale-factor in y-direction
      different values for x and y will lead to a dilation / upsetting deformation
    */
    void setScale (double factorX, double factorY) {
      AffineOp::reset (); 
      AffineOp::scale (factorX,factorY);
    }
        
    // apply should still be public
    ///applies the scale
    AffineOp::apply;

    private: // hide the following methods
    AffineOp::rotate;
    AffineOp::translate;
  };
}
#endif
