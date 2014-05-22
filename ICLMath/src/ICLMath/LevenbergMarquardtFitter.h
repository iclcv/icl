/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/LevenbergMarquardtFitter.h         **
** Module : ICLMath                                                **
** Authors: Christof Elbrechter, Sergius Gaulik                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLMath/DynVector.h>            
#include <ICLUtils/Function.h>

namespace icl{
  namespace math{
    
    
    /// Utility class implementing the multidimensional Levenberg Marquardt Algorithm for non-linear Optimization
    /** The well known Levenberg Marquardt algorithms (LMA) is used for non-linear parameter optimization.
        Given a function \f$ f : R^N \rightarrow R^O \f$, an intial set of parameters \f$ \beta_0 \f$ and 
        a set of training data \f$ \{(x_i,y_i)\}\f$, where \f$ f \f$  depends on model parameters
        \f$ \beta \f$, LMA will find a local minumum of function parameters that minimizes
        
        \f[ 
        S(\beta)​ = \sum_{o=1}^O \sum_{i=1}^m [y_i[o] - f(x_i, \ \beta)[o]​ ]​^2
        \f]
  
        The complete algorithm can be found at at wikipedia 
        (see http://en.wikipedia.org/wiki/Levenberg-Marquardt_algorithm)
  
  
        \section _F_ What type of Function can be optimized
        
        The implemented Function type, is of type icl::LevenbergMarquardtFitter::Function. It gets
        the parameters and input value and returns a vector output. Basically all functions that match
        this signature can be minimized. But actually it is quite important, that dependent on the function
        complexity, LMA will be slower of faster, or it will perhaps even get stuck in a local minimum.
        As long as the function is purely linear to the parameters (e.g)
        
        \f[ f(x) = \beta_1 + \beta_2 x + \beta_3 x^2 + beta_4 x^3 \f]
  
        Then LMA will find the solution in a single step (depened on the step parameters lambda)
        
        
        \section _J_ What is the Jacobian
  
        Due to the fact, that the LevenbergMarquardtFitter can optimized
        functions, with vector output, a set of jacobians <b>js</b> is used
        (of type std::vector<LevenbergMarquardtFitter::Jacobian>).
        <b>js[o]<b> defines the Jacobian for the o-th output dimension of f.
        Each Jacobian of \f$ f \f$ is defined by a vector (here, a
        row-vector) of partial derivations where the ith component is
        \f$ \partial f_o / \partial \beta_i\f$. The Jacobian is always
        evaluated at a given position in the input data space. For the LMA,
        the Jacobian is evaluated in each step at each given input data
        point \f$ x_i \f$ and for each output dimension. Therefore, the Jacobian 
        J_o (for output dimension o) becomes a matrix, where each row \f$ J_{oi} \f$ 
        contains the partial derivations for all parameters beta for a sigle data
        point \f$ x_i \f$ \\
        
        In the code, the Jacobian is of type
        icl::LevenbergMarquardtFitter::Jacobian. It defines how to
        compute the lines of J_o. The LevenbergMarquardtFitter class
        supports analytic and numeric Jacobians. In case of being not
        able to derive an analytic Jacobian, a numerical Jacobian can
        automatically be generated. This will estimate the real partial
        derivations by an numerical approximation.  However, it needs to
        evaluate \f$ f \f$ twice, and is usually not as exact as the
        analytic Jacobian, which howver can lead to faster or slower
        convergence.
        
        \section _EX_ Examples 
        Here, a short example is given, that shows how to use the
        LevenbergMarquardtFitter class.
        
        \subsection _EX_1_ Example 1
        The input data space and the outputspace are 1D, and our 
        function has 4 parameters (a,b,c,d) :
  
        \f[ f(x) = a + b*x + c*x^2 + d*x^3  \f]
        
        Obviously, f is linear to it's parameters. Therefore, providing an
        analytical Jacobian leads to a one-step optimiziation.
        
        The analytical Jacobian for f is 
        
        \f[ ( 1, x, x^2, x^3 ) \f]
  
        \code
        #include <ICLMath/LevenbergMarquardtFitter.h>
        
        using namespace icl::utils;
        using namespace icl::math;
        typedef float real;
        typedef icl::math::LevenbergMarquardtFitter<real> LM;
        
        LM::Vector f(const LM::Params &p, const LM::Vector &vx){
          const real x = vx[0];
          return LM::Vector(1, p[0] + p[1]*x + p[2]*x*x + p[3] * x*x*x);
        }

        void j(const LM::Params &p, const LM::Vector &vx, LM::Vector &dst){
          real x = vx[0];
          const real Ji[] = { 1, x, x*x, x*x*x };
          std::copy(Ji,Ji+4,dst.begin());
        }
        
        int main(){
          LM lm(f,1); 
          // LM lm(f,1,std::vector<LM::Jacobian>(1,j));
          const float p[4] = { 1,2,3,4 };
          LM::Params realParams(4,p), startParams(4,1);
          LM::Data data = LM::create_data(realParams,f,1,1);
          LM::Result result = lm.fit(data.x,data.y,startParams);
          SHOW(result);
        }     
        \endcode
      
      In this example, the optimization takes 7 iterations
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
        
        using namespace icl::utils;
        using namespace icl::math;
        typedef float real;
        typedef icl::math::LevenbergMarquardtFitter<real> LM;
        
        LM::Vector f3(const LM::Params &p, const LM::Vector &vx){
          real x = vx[0], y=vx[1], z=vx[2];
          real a = p[0], b=p[1], c=p[2], d=p[3], e=p[4], f=p[5];
          return LM::Vector(1,x*y*a*b + y*z*d*d*c - z*x*f*e*b + sqr(a + b + c + d));
        }

        void j3(const LM::Params &p, const LM::Vector &vx, LM::Vector &dst){
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
          LM lm(f3,1,std::vector<LM::Jacobian>(1,j3)); // use numerical jacobian LM lm(f);
          const real p[6] = { 1,2,3,4,5,6 };
          LM::Params realParams(6,p), startParams(6,3);
          LM::Data data = LM::create_data(realParams,f3,3,1);
          LM::Result result = lm.fit(data.x,data.y,startParams);
          SHOW(result);
        } 
        \endcode

        \subsection _EX_3_ Multidimensional Output
        
        The LevenbergMarquardtFitter class can also optimize functions with
        multi-dinensional output. In this case the internal optimization is
        performed for each output dimension seperately in each step. A common
        multidimensional optimization problem is 6D pose optimization. Please note,
        that there are many local minima (due to the complex relationship between
        params and output). Therefore, we need to start somewhere close to the
        original parameters. For the rotation parameters, it is always ususal to
        find different solutions that are higher or lower by n*pi or n*2pi.
        In this example, we did not try to derive an analytical
        jacobian so we use the numerical default jacobian here

        \code
        #include <ICLMath/LevenbergMarquardtFitter.h>
        #include <ICLMath/FixedVector.h>
        
        using namespace icl::utils;
        using namespace icl::math;

        typedef float real;
        typedef icl::math::LevenbergMarquardtFitter<real> LM;

        LM::Vector f4(const LM::Params &p, const LM::Vector &vx){
          FixedColVector<real,4> x(vx[0],vx[1], vx[2], 1);
          FixedMatrix<real,4,4> T = create_hom_4x4<float>(p[0],p[1],p[2],p[3],p[4],p[5]);
          FixedColVector<real,4> y = T*x;
          return LM::Vector(3,y.begin());
        }

        int main(){
          LM lm(f4,3);
          const real r[6] = { 3, 1, 2, 100, 100, 100};
          const real s[6] = { 0.1, 0.1, 0.1, 10, 10, 10 };
          LM::Params realParams(6,r), startParams(6,s);
          LM::Data data = LM::create_data(realParams,f4,3,3);
          LM::Result result = lm.fit(data.x,data.y,startParams);
          SHOW(result);
        }
        \endcode
    */
    template<class Scalar>
    class ICLMath_API LevenbergMarquardtFitter{
      public:
      
      typedef DynColVector<Scalar> Vector; //!< vector type
      typedef DynColVector<Scalar> Params; //!< parameter vector type
      typedef DynMatrix<Scalar> Matrix;    //!< matrix type (used for input data)
    
      /// Utility structure, that represents a fitting result
      struct Result{ 
        int iteration;                //!< number of iterations needed
        Scalar error;                 //!< reached error
        std::vector<Scalar> lambdas;  //!< last lambdas (one per output)
        Params params;                //!< final parameters
      
        /// overloaded ostream-operator
        friend ICLMath_API inline std::ostream &operator<<(std::ostream &str, const Result &d){
          return str << "iteration: " << d.iteration << "  error:" << d.error 
                     << "  lambda[0]:" << d.lambdas[0] << "  params:" << d.params.transp();
        }
      };

      /// utility structure that is used in the static create_data utlity method
      struct Data{
        Matrix x;  //!< input (each row is a data sample)
        Matrix y;  //!< outputs
      };

      /// to-be-optimized function type y = f(params, x)
      typedef icl::utils::Function<Vector,const Params&,const Vector&> Function;
      typedef icl::utils::Function<Matrix,const Params&,const Matrix&> FunctionMat;
    
      /// jacobian of F
      /** \see \ref _J_ */
      typedef icl::utils::Function<void,const Params&, const Vector&, Vector&> Jacobian;
      typedef icl::utils::Function<void,const Params&, const Matrix&, Matrix&> JacobianMat;
    
      /// Optionally given debug callback, that is called in every iterations
      typedef icl::utils::Function<void,const Result&> DebugCallback;
    
    

      protected:
      Function f;        //!< Function f
      FunctionMat fMat;  //!< Function f

      bool useMultiThreading;   //!< flag whether multithreading is enabled
      bool useMat;              //!< flag whether matrices in the error function
      Scalar tau;               //!< used for initial damping parameter lambda
      int maxIterations;        //!< maximum number of iterations
      Scalar minError;          //!< minimum error threshold
      Scalar lambdaMultiplier;  //!< mulitplier that is used to increase/decreas lambda
      Scalar eps1;              //!< minimum F'(parameters) threshold
      Scalar eps2    ;          //!< minimum change in parameters threshold
      std::string linSolver;    //!< linear solver that is used @see icl::DynMatrix::solve

      Vector dst;        //!< b in Mx=b of linear system solved internally
      Matrix J;          //!< Jacobian Matrix (rows are Ji)
      Matrix H;          //!< Hessian Matrix (J^T J) 

      /// output buffers
      std::vector<Jacobian> js;
      std::vector<JacobianMat> jsMat;
      
      DebugCallback dbg;  //!< debug callback
      Params params_new;  //!< new parameters (after update step)
      Matrix y_est;       //!< current estimated outputs
      //Matrix y_est_tmp;   //!< current estimated outputs (with temporary update)
      Vector dy;          //!< buffer for current delta


      /// returns the complete error
      Scalar error(const Matrix &ys, const Matrix &y_est) const;

      public:

      /// creates a dummy (null instance)
      LevenbergMarquardtFitter();
    
      /// create an instance with given parameters
      /** @param f function to be optimized 
          @param j optionally given Jacobian of f. If j is null (e.g. an empty Jacobian() is passed),
          a numerical jacobian is created automatically. This will use a numerical delta of 
          1e-5, which is the default parameter for the static 
          LevenbergMarquardtFitter::create_numerical_jacobian method. If a numerical Jacobian
          with another delta shall be used, create_numerical_jacobian can 
          be called manually to obtain another automatically created numerical
          jacobian.
          
          @param tau used for the initial damping parameter (small values (eg 1e-6) are a good choice
                 if the first parameters are believed to be a good approximation. Otherwise 1e-3 or 1 should be used)
          @param maxIterations maximum number of iterations (usually, LevenbergMarquardtFitter will converge fast, or not at all.
                 Therefore, the default argument of 200 iterations somehow assumes, that the minError
                 criterion is met much earlier.
          @param minError if the current error gets less than this threshold, the optimization is finished
          @param lambdaMultiplier mulitiplyer used for increasing/decreasing the damping parameter lambda
          @param eps1 if the F'(parameters) is less than this threshold, the algorithm is finished
          @param eps2 if the change in parameters is less than this threshold, the algorithm is finished
          @param linSolver linear solver that is used to estimate a local step internally possible values
                 are documented in icl::DynMatrix::solve (we recommend the most-stable method svd)
          */
      LevenbergMarquardtFitter(Function f, int outputDim,
          const std::vector<Jacobian> &js=std::vector<Jacobian>(),
          Scalar tau=1.e-3, int maxIterations=200,
          Scalar minError = 1.e-6, Scalar lambdaMultiplier=10,
          Scalar eps1 = 1.49012e-08, Scalar eps2 = 1.49012e-08,
          const std::string &linSolver="svd");
      LevenbergMarquardtFitter(FunctionMat f, int outputDim,
          const std::vector<JacobianMat> &js=std::vector<JacobianMat>(),
          Scalar tau=1.e-3, int maxIterations=200,
          Scalar minError = 1.e-6, Scalar lambdaMultiplier=10,
          Scalar eps1 = 1.49012e-08, Scalar eps2 = 1.49012e-08,
          const std::string &linSolver="svd");
    
      /// enables openmp based multithreading
      /** multithreading using 2 instead of 1 core provides a processing
          time reduction of about 35%. Please note, that openmp must be enabled
          using the compiler optionn "-fopenmp" as well */
      void setUseMultiThreading(bool enable);
      
      /// (re)-initialization method
      /** \copydoc LevenbergMarquardtFitter::LevenbergMarquardtFitter(Function,Jacobian,Scalar,int,Scalar,Scalar,const std::string &)*/
      void init(Function f, int outputDim,
                const std::vector<Jacobian> &js=std::vector<Jacobian>(),
                Scalar tau=1.e-8, int maxIterations=1000,
                Scalar minError = 1.e-6, Scalar lambdaMultiplier=10,
                Scalar eps1 = 1.49012e-08, Scalar eps2 = 1.49012e-08,
                const std::string &linSolver="svd");
      void init(FunctionMat f, int outputDim,
                const std::vector<JacobianMat> &js=std::vector<JacobianMat>(),
                Scalar tau=1.e-8, int maxIterations=1000,
                Scalar minError = 1.e-6, Scalar lambdaMultiplier=10,
                Scalar eps1 = 1.49012e-08, Scalar eps2 = 1.49012e-08,
                const std::string &linSolver="svd");
    
      /// actual parameter fitting with given data and start parameters
      /** This method actually fits the internal function model to the given data. The fitting
          procedure starts at the given set of initial parameters.
    
          @param xs input data matrix, where each data point is one row. Please note, that
          you can shallowly wrap a Matrix instance around existing other data types.
          @param ys output data (Scalar) ys[i] belongs to the ith row of xs
          @param initParams initial parameters for starting optimizaiton
          */
      Result fit(const Matrix &xs, const Matrix &ys, Params initParams);
   
      
    
      /// creates a single numerical Jacobian for a given function f and output dim
      /** The Jacobian will evaluate f twice for each parameter in \f$ \beta \f$. 
          Here is the internal implementation snippet:
          \code
          void jacobian(const Params &params, const Vector &x,Vector &target) const{
            Vector p = params;
            for(unsigned int i=0;i<params.dim();++i){
              p[i] = params[i] + delta/2;
              Scalar f1 = f(p,x)[o];
              p[i] = params[i] - delta/2;
              Scalar f2 = f(p,x)[o];
              p[i] = params[i];
              target[i] = ( f1 - f2 ) / delta;
            }
          }
          \endcode
          */
      static Jacobian create_numerical_jacobian(int o, Function f, float delta=1.E-5);
      static JacobianMat create_numerical_jacobian(int o, FunctionMat f, float delta=1.E-5);
      
      /// creates a set of numerical jacobians for output dimension n
      static std::vector<Jacobian> create_numerical_jacobians(int n, Function f, float delta=1.e-5);
      static std::vector<JacobianMat> create_numerical_jacobians(int n, FunctionMat f, float delta=1.e-5);

      /// creates test data using a given function
      /** @param p real function parameters
          @param f function
          @param xDim input dimension of function f
          @param num number of samples
          @param minX the function input values are randomly drawn from a uniform
          distribution in range [minX,maxX] 
          @param maxX see minX */
      static Data create_data(const Params &p, Function f, int xDim, int yDim,
                              int num=1000, Scalar minX=-5, Scalar maxX=5);

      /// default debug callback that simply streams r to std::cout
      static void default_debug_callback(const Result &r);
      
      /// sets a debug callback method, which is called automatically in every interation
      void setDebugCallback(DebugCallback dbg=default_debug_callback);

    private:
      /// internal fit function using vectors
      Result fitVec(const Matrix &xs, const Matrix &ys, Params initParams);
      /// internal fit function using matrices
      Result fitMat(const Matrix &xs, const Matrix &ys, Params initParams);
    };
  } // namespace math
} // namespace icl
