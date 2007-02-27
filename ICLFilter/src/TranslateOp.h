#ifndef TRANSLATE_OP_H
#define TRANSLATE_OP_H

#include "AffineOp.h"

namespace icl{
  
  /// Class to translate images
  class TranslateOp : public AffineOp {
    public:
    /// Constructor
    TranslateOp (double dX=0.0, double dY=0.0, scalemode eInterpolate=interpolateLIN) :
      AffineOp (eInterpolate) {}
    
    /// change rotation angle
    void setTranslation (double dX, double dY) {
      AffineOp::reset (); 
      AffineOp::translate (dX,dY);
    }
    /// apply should still be public
    AffineOp::apply;

    private: // hide the following methods
    AffineOp::rotate;
    AffineOp::scale;
  };
}
#endif
