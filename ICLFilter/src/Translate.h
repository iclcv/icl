#ifndef TRANSLATE_H
#define TRANSLATE_H

#include "Affine.h"

namespace icl{
  
  /// Class to translate images
  class Translate : private Affine {
    public:
    /// Constructor
    Translate (double dX=0.0, double dY=0.0, scalemode eInterpolate=interpolateLIN) :
      Affine (eInterpolate) {}
    
    /// change rotation angle
    void setTranslation (double dX, double dY) {
      Affine::reset (); 
      Affine::translate (dX,dY);
    }
    /// apply should still be public
    Affine::apply;
  };
}
#endif
