#ifndef ICLMEX_H
#define ICLMEX_H

#include "GeneralModel.h"

namespace icl{

  /// fits a model to given xs and ys coordinate vector 
  /** after the call, the given model reference has the found parameters.
      This parameters can easily read out with the models "[]" operator.
      @see GeneralizedModel
  */
  template<class T>
  void fitModel(T *xs, T *ys, int nPoints, GeneralModel<T> &model);
  
  /// draws a model with calculated params 
  /** The model must be fitted with the above function before it can be drawed
      into an image. 
  */
  template<class T, class X>
  void drawModel(GeneralModel<T> &model, Img<X> *image, X *color);
  
}


#endif
