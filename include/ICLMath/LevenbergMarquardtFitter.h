/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLMath/LevenbergMarquardtFitter.h             **
** Module : ICLMath                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICL_LEVENBERG_MARQUARDT_FITTER_H
#define ICL_LEVENBERG_MARQUARDT_FITTER_H

#include <ICLMath/DynVector.h>            
#include <ICLUtils/Function.h>

namespace icl{
  namespace math{
    
    
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
        
        \subsection _EX_1_ Example 1
        The input data space is 1D, and our function has 4 parameters:
  
        \f[ f(x) = a + b*x + c*x^2 + d*x^3  \f]
        
        Obviously, f is linear to it's parameters. Therefore, providing an
        analytical Jacobian leads to a one-step optimiziation.
        
        The analytical Jacobian for f is 
        
        \f[ ( 1, x, x^2, x^3 ) \f]
  
        \code
  #include <ICLMath/LevenbergMarquardtFitter.h>
  
  typedef icl::LevenbergMarquardtFitter<float> LM;
  
  float f(const LM::Params &p, const LM::Vector &vx){
    const float x = vx[0];
    return p[0] + p[1]*x + p[2]*x*x + p[3] * x*x*x;
  } // namespace math
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

      \subsection _EX_2_ Second Example

      Now we want to optimize a more complex function that has a 3D input
      data space (x,y,z) and which is no longer linear to all of it's parameters:
      The function uses 6 parameters \f$ \beta = (a,b,c,d,e,f) \f$.
      
      \f[
      f(x,y,z;\beta) =  xyab + yzd^2c - zxfeb + (a + b + c + d)^2;
      \f]
      
      Even though the function is way more complex, the analytic derivative is
      still quite easy to estimate:

      \f[ \frac{\partial f}{\partial a} = xyb + 2 (a + b + c + d)  \f]
      \f[ \frac{\partial f}{\partial b} = xya  - zxfe + 2 (a + b + c + d); \f]
      \f[ \frac{\partial f}{\partial c} = yzd^2   + 2 (a + b + c + d)\f]
      \f[ \frac{\partial f}{\partial d} = 2dyzc  + 2 (a + b + c + d)\f]
      \f[ \frac{\partial f}{\partial e} = -zxfb  \f]
      \f[ \frac{\partial f}{\partial f} = zxeb  \f]

      In case of this function using real parameters (1,2,3,4,5,6), 
      and starting parameters (3,3,3,3,3,3), we see, that the numerical Jacbobian
      does only work with double precision. If we use float precision, the analytic
      Jacobian needs to be given in order to get convergence:
      
      \code
#include <ICLMath/LevenbergMarquardtFitter.h>

typedef float real;
typedef icl::LevenbergMarquardtFitter<real> LM;

real f(const LM::Params &p, const LM::Vector &vx){
  real x = vx[0], y=vx[1], z=vx[2];
  real a = p[0], b=p[1], c=p[2], d=p[3], e=p[4], f=p[5];
  return x*y*a*b + y*z*d*d*c - z*x*f*e*b + icl::sqr(a + b + c + d);
}

void j(const LM::Params &p, const LM::Vector &vx, LM::Vector &dst){
  real x = vx[0], y=vx[1], z=vx[2];
  real a = p[0], b=p[1], c=p[2], d=p[3], e=p[4], f=p[5];
  real k = 2 * (a+b+c+d);
  dst[0] = x*y*b + k;
  dst[1] = x*y*a  - z*x*f*e + k;
  dst[2] = y*z*d*d + k;
  dst[3] = 2*d*y*z*c + k;
  dst[4] = -z*x*f*b;
  dst[5] = -z*x*e*b;
}

int main(){
  LM lm(f,j); // use numerical jacobian LM lm(f);
  const real p[6] = { 1,2,3,4,5,6 };
  LM::Params realParams(6,p), startParams(6,3);
  LM::Data data = LM::create_data(realParams,f,3);
  LM::Result result = lm.fit(data.x,data.y,startParams);
  SHOW(result);
}
\endcode
  */
  
  template<class Scalar>
  class LevenbergMarquardtFitter{
    public:
      
    typedef DynColVector<Scalar> Vector; //!< vector type
    typedef DynColVector<Scalar> Params; //!< parameter vector type
    typedef DynMatrix<Scalar> Matrix;    //!< matrix type (used for input data)
    
    /// Utility structure, that represents a fitting result
    struct Result{ 
      int iteration;  //!< number of iterations needed
      Scalar error;   //!< reached error
      Scalar lambda;  //!< last lambda
      Params params;  //!< final parameters
      
      /// overloaded ostream-operator
      friend inline std::ostream &operator<<(std::ostream &str, const Result &d){
        return str << "iteration: " << d.iteration << "  error:" << d.error 
                   << "  lambda:" << d.lambda << "  params:" << d.params.transp();
      }
    };

    /// utility structure that is used in the static create_data utlity method
    struct Data{
      Matrix x;  //!< input (each row is a data sample)
      Vector y;  //!< outputs
    };

    /// Function f that is optimized
    typedef icl::Function<Scalar,const Params&,const Vector&> Function;
    
    /// Optionally given analytical Jacobian of F
    /** \see \ref _J_ */
    typedef icl::Function<void,const Params&, const Vector&, Vector &> Jacobian;
    
    /// Optionally given debug callback, that is called in every iterations
    typedef icl::Function<void,const Result&> DebugCallback;
    
    

    protected:
    Function f;  //!< Function f
    Jacobian j;  //!< Jacobian (either analytical or numerical)

    Scalar initialLambda;     //!< initial damping parameter lambda
    int maxIterations;        //!< maximum number of iterations
    Scalar minError;          //!< minimum error threshold
    Scalar lambdaMultiplier;  //!< mulitplier that is used to increase/decreas lambda
    std::string linSolver;    //!< linear solver that is used @see icl::DynMatrix::solve

    Vector y_est;      //!< last output estimate
    Vector y_est_new;  //!< current output estimate
    Vector dst;        //!< b in Mx=b of linear system solved internally
    Matrix J;          //!< Jacobian Matrix (rows are Ji)
    Matrix H;          //!< Hessian Matrix (J^T J) 
    Matrix H_damped;   //!< Damped Hessian (H + diag(diag(H))*lambda)
    DebugCallback dbg; //!< debug callback
    Params params_new; //!< new parameters (after update step)

    public:
    
    /// creates a dummy (null instance)
    LevenbergMarquardtFitter();
    
    /// create an instance with given parameters
    /** @param f function to be optimized 
        @param j optionally given Jacobian of f. If j is null (e.g. an empty Jacobian() is passed),
                 a numerical jacobian is created automatically. This will use a numerical delta of 
                 1e-5, which is the default parameter for the static 
                 LevenbergMarquardtFitter::create_numerical_jacobian method. If a numerical Jacobian
                 with another delta shall be used, create_numerical_jacobian has to be called manually.
        @param initialLambda initial damping parameter (usually 1e-6 ist not the worst choice)
        @param maxIterations maximum number of iterations (usually, LMA will converge fast, or not at all.
               Therefore, the default argument of 10000 iterations somehow assumes, that the minError
               criterion is met much earlier.
        @param lambdaMultiplier mulitiplyer used for increasing/decreasing the damping parameter lambda
        @param linSolver linear solver that is used to estimate a local step internally possible values
               are documented in icl::DynMatrix::solve
        */
    LevenbergMarquardtFitter(Function f, Jacobian j=Jacobian(),
                            Scalar initialLambda=1.e-8, int maxIterations=10000,
                            Scalar minError = 1.e-6, Scalar lambdaMultiplier=10,
                            const std::string &linSolver="lu");
    
    /// (re)-initialization method
    /** \copydoc LevenbergMarquardtFitter::LevenbergMarquardtFitter(Function,Jacobian,Scalar,int,Scalar,Scalar,const std::string &)*/
    void init(Function f, Jacobian j=Jacobian(),
              Scalar initialLambda=1.e-8, int maxIterations=10000,
              Scalar minError = 1.e-6, Scalar lambdaMultiplier=10,
              const std::string &linSolver="lu");
    
    /// actual parameter fitting with given data and start parameters
    /** This method actually fits the internal function model to the given data. The fitting
        procedure starts at the given set of initial parameters.
    
        @param xs input data matrix, where each data point is one row. Please note, that
                  you can shallowly wrap a Matrix instance around existing other data types.
        @param ys output data (Scalar) ys[i] belongs to the ith row of xs
        @param initParams initial parameters for starting optimizaiton
    */
    Result fit(const Matrix &xs, const Vector &ys, Params initParams);
    
    
    /// creates a numerical Jacobian for a given function f
    /** The Jacobian will evaluate f twice for each parameter in \f$ \beta \f$. 
        Here is the internal implementation snippet:
        \code
        jacobian(const Params &params, const Vector &x, Vector &target) const{
           Vector p = params;
           for(unsigned int i=0;i<params.dim();++i){
              p[i] = params[i] + delta/2;
              Scalar f1 = f(p,x);
              p[i] = params[i] - delta/2;
              Scalar f2 = f(p,x);
              p[i] = params[i];
              target[i] = ( f1 - f2 ) / delta;
           }
        }
        \endcode
    */
    static Jacobian create_numerical_jacobian(Function f, float delta=1.E-5);

    /// creates test data using a given function
    /** @param p real function parameters
        @param f function
        @param xDim input dimension of function f
        @param num number of samples
        @param minX the function input values are randomly drawn from a uniform
                    distribution in range [minX,maxX] 
        @param maxX see minX */
    static Data create_data(const Params &p, Function f, int xDim, 
                            int num=1000, Scalar minX=-5, Scalar maxX=5);
    
    /// sets a debug callback method, which is called automatically in every interation
    void setDebugCallback(DebugCallback dbg);
  };
}

#endif
