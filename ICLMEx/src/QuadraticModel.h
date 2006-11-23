#ifndef QUADRATIC_MODEL_H
#define QUADRATIC_MODEL_H

#include <GeneralModel.h>

namespace icl{
 
  /// specialization of the GeneralModel class
  /** The feature function of a quadratic model must not
      produces features with a higher oder then 2. In this case,
      the Equations for x and y can be solved with the pq-Formula
      - so just the formulas for px,py and qx, and qy must be
      implemented.
      @see EllipseModel
      @see RestrictedEllipseModel
      @see CircleModel  
  */
 template<class T>
 class QuadraticModel : public GeneralModel<T>{
    public:
    virtual ~QuadraticModel(){}
    virtual Array<T> x(T y) const;
    virtual Array<T> y(T x) const;

    virtual T px(T y) const = 0; 
    virtual T qx(T y) const = 0; 
    virtual T py(T x) const = 0;     
    virtual T qy(T x) const = 0; 

    virtual void features(T x,T y, T *dst) const = 0;
    virtual void center(T &x, T &y) const = 0;

    protected:
    QuadraticModel(int dim);
 };

}
#endif
