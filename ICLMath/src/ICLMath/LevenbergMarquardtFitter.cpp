/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/LevenbergMarquardtFitter.cpp       **
** Module : ICLMath                                                **
** Authors: Christof Elbrechter                                    **
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

#include <ICLMath/LevenbergMarquardtFitter.h>
#include <ICLMath/DynMatrixUtils.h>
#include <ICLUtils/Random.h>


using namespace icl::utils;

namespace icl{
  namespace math{

    template<class Scalar>
    LevenbergMarquardtFitter<Scalar>::LevenbergMarquardtFitter(){
    
    }
    
    template<class Scalar>
    LevenbergMarquardtFitter<Scalar>::LevenbergMarquardtFitter(Function f, int outputDim,
                     const std::vector<Jacobian> &js,
                     Scalar initialLambda, int maxIterations,
                     Scalar minError, Scalar lambdaMultiplier,
                     const std::string &linSolver){
      init(f,outputDim,js,initialLambda,maxIterations,minError,lambdaMultiplier,linSolver);
    }
    
    template<class Scalar>
    void LevenbergMarquardtFitter<Scalar>::init(Function f,
                           int outputDim,
                           const std::vector<Jacobian> &js,
                           Scalar initialLambda, int maxIterations,
                           Scalar minError, Scalar lambdaMultiplier,
                           const std::string &linSolver){
      this->useMultiThreading = false;
      this->f = f;
      if(js.size()){
        this->js = js;
      }else{
        this->js = create_numerical_jacobians(outputDim,f);
      }
      this->initialLambda = initialLambda;
      this->maxIterations = maxIterations;
      this->minError = minError;
      this->lambdaMultiplier = lambdaMultiplier;
      this->linSolver = linSolver;
    }
  
    template<class Scalar>
    inline Scalar sqr_dist(const Scalar &a, const Scalar &b){
      return sqr(a-b);
    }

    template<class Scalar>
    Scalar LevenbergMarquardtFitter<Scalar>::error(const Matrix &ys, const Matrix &y_est) const {
      return ys.sqrDistanceTo(y_est)/ys.rows();
    }

    template<class Scalar>
    void LevenbergMarquardtFitter<Scalar>::setUseMultiThreading(bool enable){
      useMultiThreading = enable;
    }
    
    template<class Scalar>
    typename LevenbergMarquardtFitter<Scalar>::Result
    LevenbergMarquardtFitter<Scalar>::fit(const Matrix &xs, const Matrix &ys, Params params){
      const int O = ys.cols(); // output dim
      ICLASSERT_THROW(O == (int)js.size(), ICLException("LevenbergMarquardtFitter::fit: ys.cols() and outputDim differ"));
      const int I = xs.cols();
      const int D = xs.rows();
      const int P = params.dim();
      const int MAX_IT = maxIterations;
      const Scalar MIN_E = minError;
      
      dst.setDim(P);
      
      J.setBounds(P,D);
      H.setBounds(P,P);
      H.setBounds(P,P);
      
      params_new.setDim(P); 
      y_est.setBounds(O,D);
      dy.setBounds(D);

      std::vector<Vector> xs_rows(D);
      std::vector<Vector> ys_rows(D);
      std::vector<Vector> y_est_rows(D);
      
      for(int i=0;i<D;++i){
        xs_rows[i] = Vector(I, const_cast<Scalar*>(xs.row_begin(i)),false);
        ys_rows[i] = Vector(O, const_cast<Scalar*>(ys.row_begin(i)),false);
        y_est_rows[i] = Vector(O, y_est.row_begin(i), false);
        y_est_rows[i] = f(params,xs_rows[i]);
      }

      std::vector<Scalar> lambdas(O,initialLambda);
      Scalar e = error(ys,y_est);
      if(e < minError){
        Result r = {-1, e, lambdas, params};
        if(dbg) dbg(r);
        return r;
      }
      
      int it = 0;
      
#ifdef USE_OPENMP
      bool mt = useMultiThreading;
#endif
      
      for(;it < MAX_IT; ++it){
        for(int o=0;o<O;++o){

#ifdef USE_OPENMP
#pragma omp parallel for if(mt)
#endif
          for(int i=0;i<D;++i){
            Vector Ji(P,J.row_begin(i),false);
            js[o](params,xs_rows[i],Ji);
            dy[i] = f(params, xs_rows[i])[o] - ys(o,i);
          }
          
          matrix_mult_t(J,J,H,SRC1_T);
          matrix_mult_t(J,dy,dst,SRC1_T); 
          
          for(int i=0;i<P;++i){
            H(i,i) *= (1.f + lambdas[o]);
          }

          // Creating Mx = b to solve
          // (H + lambda diag (H)) x = J^T(y-f(beta))
          Params pSolved = H.solve(dst,"svd");

          params_new = params - pSolved;

#ifdef USE_OPENMP
#pragma omp parallel for if(mt)
#endif
          for(int i=0;i<D;++i){
            y_est_rows[i] = f(params_new,xs_rows[i]); 
          }          
          Scalar e_new = error(ys, y_est);
        
          if(e_new < e){
            if(e_new < MIN_E){
              Result result = { it, e_new, lambdas, params_new};
              return result;
            }
            e = e_new;
            lambdas[o] /= lambdaMultiplier;
            params = params_new;
          }else{
            lambdas[o] *= lambdaMultiplier;
            if(lambdas[o] > 10.e30) {
              Result r = { it, e, lambdas, params };
              if(dbg) dbg(r);
              return r;
            }
          }
          if(dbg){
            Result r = { it, e, lambdas, params};
            dbg(r);
          }
        }
      }
      Result result = { it, e, lambdas, params };
      return result;
    }
  
    
    namespace{
      template<class Scalar>
      struct NumericJacobian : public FunctionImpl<void,const DynColVector<Scalar>&,
      const DynColVector<Scalar>&,
      DynColVector<Scalar>&>{
        typedef typename LevenbergMarquardtFitter<Scalar>::Function Function;
        typedef typename LevenbergMarquardtFitter<Scalar>::Params Params;
        typedef typename LevenbergMarquardtFitter<Scalar>::Vector Vector;
        
        int o;      
        Function f;
        Scalar delta;
        
        NumericJacobian(int o, Function f, Scalar delta):
          o(o),f(f),delta(delta){}
        
        virtual void operator()(const Params &params, 
                                const Vector &x,
                                Vector &target) const{
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
      };
    }
  
  
    template<class Scalar>
    typename LevenbergMarquardtFitter<Scalar>::Jacobian 
    LevenbergMarquardtFitter<Scalar>::create_numerical_jacobian(int o, Function f, float delta){
      return Jacobian(new NumericJacobian<Scalar>(o,f,delta));
    }

    template<class Scalar>
    std::vector<typename LevenbergMarquardtFitter<Scalar>::Jacobian> 
    LevenbergMarquardtFitter<Scalar>::create_numerical_jacobians(int n, Function f, float delta){
      std::vector<typename LevenbergMarquardtFitter<Scalar>::Jacobian> js(n);
      for(int i=0;i<n;++i){
        js[i] = create_numerical_jacobian(i,f);
      }
      return js;
    }

    
    template<class Scalar>
    void LevenbergMarquardtFitter<Scalar>::setDebugCallback(DebugCallback dbg){
      this->dbg = dbg;
    }
    
    template<class Scalar>
    typename LevenbergMarquardtFitter<Scalar>::Data
    LevenbergMarquardtFitter<Scalar>::create_data(const Params &p, Function f, int xDim, int yDim, int num, Scalar minX, Scalar maxX){
      URand r(minX,maxX);
      Data data = { Matrix(xDim,num), Matrix(yDim, num) };
      for(int i=0;i<num;++i){
        std::fill(data.x.row_begin(i), data.x.row_end(i), r);
        Vector yi(yDim, data.y.row_begin(i), false);
        yi = f(p,Vector(xDim,data.x.row_begin(i),false));
      }
      return data;
    }

    template<class Scalar>
    void LevenbergMarquardtFitter<Scalar>::default_debug_callback(const Result &r){
      std::cout << r << std::endl;
    }

  
    template class ICL_MATH_API LevenbergMarquardtFitter<icl32f>;
    template class ICL_MATH_API LevenbergMarquardtFitter<icl64f>;
  } // namespace math
}

