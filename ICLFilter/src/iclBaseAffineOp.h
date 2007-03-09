#include "iclUnaryOp.h"
#ifndef BASEAFFINE_OP_H
#define BASEAFFINE_OP_H


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
  class BaseAffineOp : public UnaryOp{
    public:
    virtual ~BaseAffineOp(){}
  };

}

#endif
