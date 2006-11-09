#ifndef CIRCLE_MODEL_H
#define CIRCLE_MODEL_H


#include <QuadraticModel.h>

namespace icl{

  /// Circle specific model
  /** the underlyinf equation for the circle is
      <pre>
      a(x²+y²) + bx + cy + d = 0 
      </pre>
  */ 
 template<class T>
 class CircleModel : public QuadraticModel<T>{
    public:
    CircleModel();
    virtual ~CircleModel(){}

    virtual T px(T y, T* params) const;
    virtual T qx(T y, T* params) const;
    virtual T py(T x, T* params) const;
    virtual T qy(T x, T* params) const;

    virtual void features(T x,T y, T *dst)const;
 };

}

#endif
