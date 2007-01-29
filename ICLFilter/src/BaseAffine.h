#ifndef BASEAFFINE_H
#define BASEAFFINE_H

#include "Filter.h"
#include "ICLTypes.h"

namespace icl{

  /// Abtract base class for arbitrary affine operation classes
  /** The Base affine class complies an abtract interface class
      for all Filter classes implementing affine operations:
      - Affine (General Affine Transformations using 3x2 Matrix)
      - Rotate
      - Translate
      - Mirror
      - Scale  
  */
  class BaseAffine : public Filter{
    public:
    virtual ~BaseAffine(){}
    /// pure virtual apply function implemented in the certain implementations of BaseAffine
    virtual void apply(const ImgBase *src, ImgBase **dst)=0;
  };

}

#endif
