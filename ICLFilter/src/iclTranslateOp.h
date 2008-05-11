#ifndef TRANSLATE_OP_H
#define TRANSLATE_OP_H

#include "iclAffineOp.h"

namespace icl{
  
  /// Class to translate images \ingroup UNARY \ingroup AFFINE
  class TranslateOp : public AffineOp {
    public:
    /// Constructor
    TranslateOp (double dX=0.0, double dY=0.0, scalemode eInterpolate=interpolateLIN) :
      AffineOp (eInterpolate) {
        setTranslation(dX,dY);
      }
    
    /// performs a translation
    /**
      @param dX pixels to translate in x-direction
      @param dY pixels to translate in y-direction
    */

    void setTranslation (double dX, double dY) {
      AffineOp::reset (); 
      AffineOp::translate (dX,dY);
    }
    
    // apply should still be public
    
    ///applies the translation
    AffineOp::apply;

    private: // hide the following methods
    AffineOp::rotate;
    AffineOp::scale;
  };
}
#endif
