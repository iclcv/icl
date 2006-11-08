#ifndef GENERAL_MODEL_H
#define GENERAL_MDOEL_H

#include "Array.h" 
#include "Img.h"

namespace icl{

  /// GeneralModel class 
  /** A general Model is a formalisation of a model that should be
      extracted from a set of found image x and y points.

  <pre>
      A model has the form: F(A,X)=0

      where A={a,b,c,d,..} is a parameter vector and 
            X={f1(x),f2(x),...} is a set of functions on the input data space
            here x=(x,y)
  
      an elliptic model for example can be parameterized as follows:
      F(A,X) = ax² + bxy + cy² + dx + ey + f = 0;

      If the parameter b for the mixed term is left out, the ellipses major
      axis are parallel the x and y axis of the 
      F(A,X) = ax² + by² + cx + dy + e = 0;

      A further restriction does not allow the "ellipse" to have different
      length of the major axis, so the model becomes a circle-model:
      F(A,X) = a(x² + y²) + bx + cy + d = 0;

      Also linear models can be formalized, so the following model describes
      an arbitrary line.
      F(A,X) = ax + by + c = 0;

      If further the bias c is left out, the line has to hit the origin.
      So the vector A=(a,b) is the normal vector of the line:
      F(A,X) = ax + by = 0;
  
      These models can be fit into a set of datapoints using a regression
      aproach: xs={x1,x2,...,xn} and ys={y1,y2,...,yn} be the data point vectors,
      then      F(x1 
            D = 

      ...

  </pre>
  
  */
  template<class T>
  class GeneralModel{
    public:
    virtual ~GeneralModel(){}
    virtual void features(T x,T y, T *dst) const =0;
    virtual Array<T> x(T y, T *params) const = 0;
    virtual Array<T> y(T x, T *params) const = 0;

    int dim()const;
    const T *constraints()const;
    void setIdentityConstraintMatrix();
    
    protected:
    GeneralModel(int dim);
    
    Array<T> m_oConstraintMatrix;
    int m_iDim;
  };
   

    
}
#endif
