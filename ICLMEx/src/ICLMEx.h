#ifndef ICLMEX_H
#define ICLMEX_H

#include "GeneralModel.h"

namespace icl{

  /// fits a model to given xs and ys coordinate vector 
  /** The size of dstParams must be at least the models dim()  
  */
  template<class T>
  void fitModel(T *xs, T *ys, int nPoints, const GeneralModel<T> &model, T *dstParams);
  
  /// draws a model with calculated params 
  /** The size of dstParams must be at least the models dim(), the color
      must have at least the size of the images channel count.
  */
  template<class T, class X>
  void drawModel(GeneralModel<T> &model, Img<X> *image, T *params, X *color);
  
}


#endif
