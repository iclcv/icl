#include <iclQuadraticModel.h>
#ifndef CIRCLE_MODEL_H
#define CIRCLE_MODEL_H



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

    virtual T px(T y) const;
    virtual T qx(T y) const;
    virtual T py(T x) const;
    virtual T qy(T x) const;

    virtual void features(T x,T y, T *dst)const;
    virtual void center(T &x, T &y) const;
 };

}

#endif
