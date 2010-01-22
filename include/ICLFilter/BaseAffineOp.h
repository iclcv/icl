#ifndef BASEAFFINE_OP_H
#define BASEAFFINE_OP_H

#include <ICLFilter/UnaryOp.h>

namespace icl{

  /// Abtract base class for arbitrary affine operation classes \ingroup AFFINE \ingroup UNARY
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
    /// Destructor
    virtual ~BaseAffineOp(){}
    
    /// import from super class
    UnaryOp::apply;
  };

}

#endif
