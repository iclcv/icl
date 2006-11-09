#ifndef RESTRICTED_ELLIPSE_MODEL_H
#define RESTRICTED_ELLIPSE_MODEL_H


#include <QuadraticModel.h>

namespace icl{


  /// Ellipse specific model with no mixed term 
  /** the underlyinf equation for the ellise is
      <pre>
      ax² + by² + cx + dy + e = 0 
      </pre>
      These ellises are not rotated
  */ 
 template<class T>
 class RestrictedEllipseModel : public QuadraticModel<T>{
    public:
    RestrictedEllipseModel();
    virtual ~RestrictedEllipseModel(){}

    virtual T px(T y, T* params) const;
    virtual T qx(T y, T* params) const;
    virtual T py(T x, T* params) const;
    virtual T qy(T x, T* params) const;

    virtual void features(T x,T y, T *dst)const;
 };

}

#endif
