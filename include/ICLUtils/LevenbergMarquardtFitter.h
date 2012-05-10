#ifndef ICL_LEVENBERG_MARQUARDT_FITTER_H
#define ICL_LEVENBERG_MARQUARDT_FITTER_H

#include <ICLUtils/DynVector.h>            
#include <ICLUtils/Function.h>

namespace icl{
  
  
  /// Utility class implementing the Levenberg Marquardt Algorithm for non-linear Optimization
  /** The well known Levenberg Marquardt algorithms (LMA) is used for non-linear parameter optimization.
      Given a function \f$ f : R^N \rightarrow R \f$, an intial set of parameters \f$ \beta_0 \f$ and 
      a set of training data \f$ \{(x_i,y_i)\}\f$, where \f$ f \f$  depends on model parameters
      \f$ \beta \f$, LMA will find a local minumum of function parameters that minimizes
      
      \f[ 
      S(\beta)​ = \sum_{i=1}^m [y_i - f(x_i, \ \beta)​ ]​^2
      \f]

      The complete algorithm can be found at http://en.wikipedia.org/wiki/Levenberg-Marquardt_algorithm


      \section _F_ What type of Function can be optimized
      
      The implemented Function type, is of type icl::LevenbergMarquardtFitter::Function. It gets
      the parameters and input value and returns a scalar output. Basically all functions that match
      this signature can be minimized. But actually it is quite important, that dependent on the function
      complexity, LMA will be slower of faster, or it will perhaps even get stuck in a local minimum.
      As long as the function is purely linear to the parameters (e.g)
      
      \f[ f(x) = \beta_1 + \beta_2 x + \beta_3 x^2 + beta_4 x^3 \f]

      Then LMA will find find the solution in a single step (depened on the step parameters lambda)
      
      
      \section _J_ What is the Jacobian

      The Jacobian of \f$ f \f$ is defined by a vector (here, a
      row-vector) of partial derivations where the ith component is
      \f$\partial f / \partial \beta_i\f$. The Jacobian is always
      evaluated at a position in the input data space. For the LMA,
      the Jacobian is evaluated in each step at each given input data
      point \f$ x_i \f$. Therefore, the Jacobian J becomes a matrix,
      where each row \f$ J_i \f$ contains the partial derivations for
      all parameters beta for a sigle data pointer \f$ x_i \f$.\\
      
      In the code, the Jacobian is of type
      icl::LevenbergMarquardtFitter::Jacobian. It defines how to
      compute the lines of J. The LevenbergMarquardtFitter class
      supports analytic and numeric Jacobians. In case of being not
      able to derive an analytic Jacobian, a numerical Jacobian can
      automatically be generated. This will estimate the real partial
      derivations by an numeric approximation.  However, it needs to
      evaluate \f$ f \f$ twice, and is usually not as exact as the
      analytic Jacobian.
      
      \section _EX_ Examples 
      Here, a short example is given, that shows how to use the
      LevenbergMarquardtFitter class.
      
      \section _EX_1_ Example 1
      The input data space is 1D, and our function has 4 parameters:

      \f[ f(x) = a + b*x + c*x^2 + d*x^3  \f]
      
      Obviously, f is linear to it's parameters. Therefore, providing an
      analytical Jacobian leads to a one-step optimiziation.
      
      The analytical Jacobian for f is 
      
      \f[ ( 1, x, x^2, x^3 ) \f]

      \code
#include <ICLUtils/LevenbergMarquardtFitter.h>

typedef icl::LevenbergMarquardtFitter<float> LM;

float f(const LM::Params &p, const LM::Vector &vx){
  const float x = vx[0];
  return p[0] + p[1]*x + p[2]*x*x + p[3] * x*x*x;
}

void j(const LM::Params &p, const LM::Vector &vx, LM::Vector &dst){
  float x = vx[0];
  const float Ji[] = { 1, x, x*x, x*x*x };
  std::copy(Ji,Ji+4,dst.begin());
}

int main(){
  LM lm(f); // use analytic jacobian LM lm(f,j)
  const float p[4] = { 1,2,3,4 };
  LM::Params realParams(4,p), startParams(4,1);
  LM::Data data = LM::create_data(realParams,f,1);
  LM::Result result = lm.fit(data.x,data.y,startParams);
  SHOW(result);
}
      \endcode
      
      In this example, the optimization takes 11 iterations
      if the numerical Jacobian is used. The Analytic Jacobian
      leads to a much better convergence and leads to a one-step
      fitting. If doubles instead of floats are used, the numerical
      Jacobian is also accurate enough for one-step fitting.
  */
  
  template<class Scalar>
  class LevenbergMarquardtFitter{
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

    struct Data{
      Matrix x;
      Vector y;
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
    LevenbergMarquardtFitter();
    
    LevenbergMarquardtFitter(Function f, Jacobian j=Jacobian(),
                            Scalar initialLambda=1.e-8, int maxIterations=10000,
                            Scalar minError = 1.e-6, Scalar lambdaMultiplier=10,
                            const std::string &linSolver="lu");
    
    void init(Function f, Jacobian j=Jacobian(),
              Scalar initialLambda=1.e-8, int maxIterations=10000,
              Scalar minError = 1.e-6, Scalar lambdaMultiplier=10,
              const std::string &linSolver="lu");
    
    Result fit(const Matrix &xs, const Vector &ys, Params initParams);
    
    static Jacobian create_numerical_jacobian(Function f, float delta=1.E-5);
    
    static Data create_data(const Params &p, Function f, int xDim, 
                            int num=1000, Scalar minX=-5, Scalar maxX=5);

    void setDebugCallback(DebugCallback dbg);
  };
}

#endif
