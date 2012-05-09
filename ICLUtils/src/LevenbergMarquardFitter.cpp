#include <ICLUtils/LevenbergMarquardFitter.h>

namespace icl{
  template<class Scalar>
  LevenbergMarquardFitter<Scalar>::LevenbergMarquardFitter(){
  
  }
  
  template<class Scalar>
  LevenbergMarquardFitter<Scalar>::LevenbergMarquardFitter(Function f, Jacobian j, 
                                                           int initialLambda, int maxIterations,
                                                           Scalar minError, const std::string &linSolver){
    init(f,j,initialLambda,maxIterations,minError,linSolver);
  }

  template<class Scalar>
  void LevenbergMarquardFitter<Scalar>::init(Function f, Jacobian j,
                                             int initialLambda, int maxIterations,
                                             Scalar minError, const std::string &linSolver){
    this->f = f;
    if(j){
      this->j = j;
    }else{
      this->j = create_numeric_jacobian(f);
    }
    this->initLambda = initialLambda;
    this->maxIterations = maxIterations;
    this->minError = minError;
    this->linSolver = linSolver;
  }

  
  template<class Scalar>
  typename LevenbergMarquardFitter<Scalar>::Params 
  LevenbergMarquardFitter<Scalar>::fit(const Matrix &xs, const Vector &ys, Params params){
    const int I = xs.cols();
    const int D = xs.rows();
    const int P = params.dim();
    const int MAX_IT = maxIterations;
    const Scalar MIN_E = minError;
    
    y_est.setDim(D);
    y_est_new.setDim(D);
    dst.setDim(D);
    
    J.setBounds(P,D);
    H.setBounds(P,P);
    H_damped.setBounds(P,P);
    
    Scalar lambda=initLambda;
    Scalar e = 1e38;
    Scalar e_new = 0;
    bool dirty=true;
    Params params_new(P); 
    
    std::vector<Vector> xis(D);
    for(int i=0;i<D;++i){
      xis[i] = Vector(I,const_cast<Scalar*>(xs.row_begin(i)),false);
    }
    
    for(int it=0;it < MAX_IT; ++it){
      if(dirty){
        for(int i=0;i<D;++i){
          Vector Ji(P,J.row_begin(i),false);
          j(params,xis[i],Ji);
          y_est[i] = f(params,xis[i]);
        }
        matrix_mult_t(J,J,H,SRC1_T); // H = J.transp() * J;
        matrix_mult_t(J,y_est-ys,dst,SRC1_T); 
      }
      
      H_damped = H;
      for(int i=0;i<P;++i){
        H_damped(i,i) += H_damped(i,i) * lambda;
      }
      // Creating Mx = b to solve
      // (H + lambda diag (H)) x = J^T(y-f(beta))
      params_new = params - H_damped.solve(dst,linSolver);
      
      e_new = 0;
      for(int i=0;i<D;++i){
        y_est_new[i] = f(params_new, xis[i]);
        e_new += sqr(ys[i]-y_est_new[i]);
      }
      
      if(e_new < e){
        if(e_new < MIN_E){
          return params_new;
        }
        e = e_new;
        lambda /= 2;
        params = params_new;
        dirty = true;
      }else{
        dirty=false;
        lambda *= 2;
      }
    }
    return params_new;
  }

  
  template<class Scalar>
  struct NumericJacobian : public FunctionImpl<void,const DynColVector<Scalar>&,const DynColVector<Scalar>&,DynColVector<Scalar>&>{
    typedef typename LevenbergMarquardFitter<Scalar>::Function Function;
    typedef typename LevenbergMarquardFitter<Scalar>::Params Params;
    typedef typename LevenbergMarquardFitter<Scalar>::Vector Vector;
    
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
        target[i] = ( f1 - f2 ) / delta;
      }
    }
  };


  template<class Scalar>
  typename LevenbergMarquardFitter<Scalar>::Jacobian 
  LevenbergMarquardFitter<Scalar>::create_numeric_jacobian(Function f, float delta){
    return Jacobian(new NumericJacobian<Scalar>(f,delta));
  }

  template class LevenbergMarquardFitter<icl32f>;
  template class LevenbergMarquardFitter<icl64f>;

}
