#ifndef BINARY_OPERATOR_H
#define BINARY_OPERATOR_H

#include "ICLTypes.h"

/// Abstract base class for operators of type result=f(imageA,imageB)
/** <b>TODO!!!</b> clip to ROI and prepare logic here !*/
class BinaryOperator{
  public:
  /// pure virtual apply function
  virtual void apply(ImgBase *operand1, ImgBase *operand2, ImgBase **result)=0;
};


#endif
