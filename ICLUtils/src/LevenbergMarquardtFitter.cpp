/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/LevenbergMarquardtFitter.cpp              **
** Module : ICLUtils                                               **
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

#include <ICLUtils/LevenbergMarquardtFitter.h>
#include <ICLUtils/DynMatrixUtils.h>
#include <ICLUtils/Random.h>

namespace icl{
  template<class Scalar>
  LevenbergMarquardtFitter<Scalar>::LevenbergMarquardtFitter(){
  
  }
  
  template<class Scalar>
  LevenbergMarquardtFitter<Scalar>::LevenbergMarquardtFitter(Function f, Jacobian j, 
                                                           Scalar initialLambda, int maxIterations,
                                                           Scalar minError, Scalar lambdaMultiplier,
                                                           const std::string &linSolver){
    init(f,j,initialLambda,maxIterations,minError,lambdaMultiplier,linSolver);
  }

  template<class Scalar>
  void LevenbergMarquardtFitter<Scalar>::init(Function f, Jacobian j,
                                             Scalar initialLambda, int maxIterations,
                                             Scalar minError, Scalar lambdaMultiplier,
                                             const std::string &linSolver){
    this->f = f;
    if(j){
      this->j = j;
    }else{
      this->j = create_numerical_jacobian(f);
    }
    this->initialLambda = initialLambda;
    this->maxIterations = maxIterations;
    this->minError = minError;
    this->lambdaMultiplier = lambdaMultiplier;
    this->linSolver = linSolver;
  }

  
  template<class Scalar>
  typename LevenbergMarquardtFitter<Scalar>::Result
  LevenbergMarquardtFitter<Scalar>::fit(const Matrix &xs, const Vector &ys, Params params){
    const int I = xs.cols();
    const int D = xs.rows();
    const int P = params.dim();
    const int MAX_IT = maxIterations;
    const Scalar MIN_E = minError;
    
    y_est.setDim(D);
    y_est_new.setDim(D);
    dst.setDim(D);
    params_new.setDim(P); 
    
    J.setBounds(P,D);
    H.setBounds(P,P);
    H_damped.setBounds(P,P);
    H.setBounds(P,P);

    Scalar lambda=initialLambda;
    Scalar e = 1e38;
    Scalar e_new = 0;
    bool dirty=true;
    
    std::vector<Vector> xis(D);

    for(int i=0;i<D;++i){
      xis[i] = Vector(I,const_cast<Scalar*>(xs.row_begin(i)),false);
    }
    
    int it = 0;
    for(;it < MAX_IT; ++it){
      if(dirty){
        for(int i=0;i<D;++i){
          Vector Ji(P,J.row_begin(i),false);
          j(params,xis[i],Ji);
          y_est[i] = f(params,xis[i]);
        }
        matrix_mult_t(J,J,H,SRC1_T); 
        matrix_mult_t(J,y_est-ys,dst,SRC1_T); 
      }
      
      H_damped = H;
      for(int i=0;i<P;++i){
        H_damped(i,i) += H_damped(i,i) * lambda;
      }
      // Creating Mx = b to solve
      // (H + lambda diag (H)) x = J^T(y-f(beta))
      Params pSolved = H_damped.solve(dst,linSolver);
      params_new = params - pSolved;
      
      e_new = 0;

      for(int i=0;i<D;++i){
        //  SHOW(xis[i])
        y_est_new[i] = f(params_new, xis[i]);
        e_new += sqr(ys[i]-y_est_new[i]);
        // std::cout << "ys["<<i<<"] = " << ys[i] << "   --> y_est["<<i<<"]" << y_est_new[i] << std::endl;
      }
      
      
      if(e_new < e){
        if(e_new < MIN_E){
          Result result = { it, e_new, lambda, params_new};
          return result;
        }
        e = e_new;
        lambda /= lambdaMultiplier;
        params = params_new;
        dirty = true;
      }else{
        dirty=false;
        lambda *= lambdaMultiplier;
      }
      if(dbg){
        Result r = { it, e, lambda, params};
        dbg(r);
      }
    }
    Result result = { it, e, lambda, params };
    return result;
  }

  
  template<class Scalar>
  struct NumericJacobian : public FunctionImpl<void,const DynColVector<Scalar>&,const DynColVector<Scalar>&,DynColVector<Scalar>&>{
    typedef typename LevenbergMarquardtFitter<Scalar>::Function Function;
    typedef typename LevenbergMarquardtFitter<Scalar>::Params Params;
    typedef typename LevenbergMarquardtFitter<Scalar>::Vector Vector;
    
    Function f;
    Scalar delta;
    
    NumericJacobian(Function f, Scalar delta):
      f(f),delta(delta){}
    
    virtual void operator()(const Params &params, 
                            const Vector &x,
                            Vector &target) const{
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
  };


  template<class Scalar>
  typename LevenbergMarquardtFitter<Scalar>::Jacobian 
  LevenbergMarquardtFitter<Scalar>::create_numerical_jacobian(Function f, float delta){
    return Jacobian(new NumericJacobian<Scalar>(f,delta));
  }
  
  template<class Scalar>
  void LevenbergMarquardtFitter<Scalar>::setDebugCallback(DebugCallback dbg){
    this->dbg = dbg;
  }
  
  template<class Scalar>
  typename LevenbergMarquardtFitter<Scalar>::Data
  LevenbergMarquardtFitter<Scalar>::create_data(const Params &p, Function f, int xDim, int num, Scalar minX, Scalar maxX){
    URand r(minX,maxX);
    Data data = { Matrix(xDim,num), Vector(num) };
    for(int i=0;i<num;++i){
      data.x[i] = r;
      data.y[i] = f(p,Vector(xDim,data.x.row_begin(i),false));
    }
    return data;
  }

  template class LevenbergMarquardtFitter<icl32f>;
  template class LevenbergMarquardtFitter<icl64f>;

}

