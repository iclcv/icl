#ifndef ICL_LEVENBERG_MARQUARD_FITTER_H
#define ICL_LEVENBERG_MARQUARD_FITTER_H

#include <ICLQuick/Common.h>
#include <ICLUtils/DynVector.h>            
#include <ICLUtils/DynMatrixUtils.h>
#include <ICLUtils/Random.h>

namespace icl{
  template<class Scalar>
  class LevenbergMarquardFitter{
    public:
    typedef DynColVector<Scalar> Vector;
    typedef DynColVector<Scalar> Params;
    typedef DynMatrix<Scalar> Matrix;

    typedef icl::Function<Scalar,const Params&,const Vector&> Function;
    typedef icl::Function<void,const Params&, const Vector&, Vector &> Jacobian;

    protected:
    Function f;    
    Jacobian j;
    Scalar initLambda;
    int maxIterations;
    Scalar minError;
    std::string linSolver;

    Vector y_est,y_est_new, dst;
    Matrix J,H,H_damped;

    public:
    LevenbergMarquardFitter();
    
    LevenbergMarquardFitter(Function f, Jacobian j=0, 
                            int initialLambda=1.e-6, int maxIterations=10000,
                            Scalar minError = 1.e-6, const std::string &linSolver="lu");
    
    void init(Function f, Jacobian j=0, 
              int initialLambda=1.e-6, int maxIterations=10000,
              Scalar minError = 1.e-6, const std::string &linSolver="lu");
    
    Params fit(const Matrix &xs, const Vector &ys, Params initParams);
    
    static Jacobian create_numeric_jacobian(Function f, float delta=1.E-5);
  };
}

#endif
