#ifndef ICL_LEVENBERG_MARQUARD_FITTER_H
#define ICL_LEVENBERG_MARQUARD_FITTER_H

#include <ICLUtils/DynVector.h>            
#include <ICLUtils/Function.h>

namespace icl{
  template<class Scalar>
  class LevenbergMarquardFitter{
    public:
      
    typedef DynColVector<Scalar> Vector;
    typedef DynColVector<Scalar> Params;
    typedef DynMatrix<Scalar> Matrix;

    struct Result{
      int iteration;
      Scalar error;
      Scalar lambda;
      Params params;
      friend inline std::ostream &operator<<(std::ostream &str, const Result &d){
        return str << "iteration: " << d.iteration << "  error:" << d.error 
                   << "  lambda:" << d.lambda << "  params:" << d.params.transp();
      }
    };

    typedef icl::Function<Scalar,const Params&,const Vector&> Function;
    typedef icl::Function<void,const Params&, const Vector&, Vector &> Jacobian;
    typedef icl::Function<void,const Result&> DebugCallback;

    protected:
    Function f;    
    Jacobian j;
    Scalar initialLambda;
    int maxIterations;
    Scalar minError;
    Scalar lambdaMultiplier;
    std::string linSolver;

    Vector y_est,y_est_new, dst;
    Matrix J,H,H_damped;
    DebugCallback dbg;
    Params params_new;

    public:
    LevenbergMarquardFitter();
    
    LevenbergMarquardFitter(Function f, Jacobian j=Jacobian(), 
                            Scalar initialLambda=1.e-8, int maxIterations=10000,
                            Scalar minError = 1.e-6, Scalar lambdaMultiplier=10,
                            const std::string &linSolver="lu");
    
    void init(Function f, Jacobian j=Jacobian(), 
              Scalar initialLambda=1.e-8, int maxIterations=10000,
              Scalar minError = 1.e-6, Scalar lambdaMultiplier=10,
              const std::string &linSolver="lu");
    
    Result fit(const Matrix &xs, const Vector &ys, Params initParams);
    
    static Jacobian create_numeric_jacobian(Function f, float delta=1.E-5);
    
    void setDebugCallback(DebugCallback dbg);
  };
}

#endif
