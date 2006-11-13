#ifndef ELLIPSE_MODEL_H
#define ELLIPSE_MODEL_H


#include <QuadraticModel.h>

namespace icl{

  /// Ellipse specific model
  /** the underlyinf equation for the ellise is
      <pre>
      ax² + bxy + cy² + dx + ey + f = 0 
      </pre>
  */
 template<class T>
 class EllipseModel : public QuadraticModel<T>{
    public:
    EllipseModel();
    virtual ~EllipseModel(){}

    virtual T px(T y) const;
    virtual T qx(T y) const;
    virtual T py(T x) const;
    virtual T qy(T x) const;

    virtual void features(T x,T y, T *dst)const;
 };

}

#endif
