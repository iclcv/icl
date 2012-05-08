#include <ICLQuick/Common.h>
#include <ICLUtils/DynVector.h>            
#include <ICLUtils/DynMatrixUtils.h>
#include <ICLUtils/Random.h>

typedef double real;
typedef DynColVector<real> V;
typedef DynMatrix<real> M;
typedef V Params;
struct XY {
  V x;
  real y;
};

Params lma( const std::vector<XY> &data,
            Function<real,const Params&,const V&> f, 
            Function<V,const Params&,const V&> j,
            Params params){
  real lambda=0.000001;
  bool updateJ=true;
  const int I = data[0].x.cols();
  const int D = data.size();
  const int P = params.dim();
  const int MAX_IT = 10;
  const real MIN_E = 1e-20;
  
  V y(D),y_est(D),y_est_new(D), dst(D);
  M J(P,D), H(P,P), H_damped(P,P);
  
  Params params_new(P); 
  real e = 1e38, e_new(0);
  
  for(int i=0;i<D;++i){
    y[i] = data[i].y;
  }

  for(int it=0;it < MAX_IT; ++it){
    std::cout << "iteratiion " << it << " current error is " << e << std::endl; 
    if(updateJ){
      for(int i=0;i<D;++i){
        J.row(i) = j(params,data[i].x);
        y_est[i] = f(params, data[i].x); // d = y_est ??
      }
      matrix_mult_t(J,J,H,SRC1_T); // H = J.transp() * J;
      matrix_mult_t(J,y_est-y,dst,SRC1_T); 
    }
    
    H_damped = H;
    for(int i=0;i<P;++i){
      H_damped(i,i) += H_damped(i,i) * lambda;
    }
    // Creating Mx = b to solve
    // (H + lambda diag (H)) x = J^T(y-f(beta))
    
    params_new = params - H_damped.solve(dst);
    SHOW(params_new.transp());

    e_new = 0;
    for(int i=0;i<D;++i){
      y_est_new[i] = f(params_new, data[i].x); // d = y_est ??
      e_new += sqr(y[i]-y_est_new[i]);
    }
    
    if(e_new < e){
      if(e_new < MIN_E){
        return params_new;
      }
      e = e_new;
      lambda *= 2;
      params = params_new;
      updateJ = true;
    }else{
      updateJ=false;
      lambda /= 2;
    }
  }
  return params_new;
}

// dim x = 1
real f(const Params &p, const V &vx){
  const real x = vx[0];
  return p[0] + p[1]*x + p[2]*x*x + p[3] * x*x*x;
}
V j(const Params &p, const V &vx){
  const real x = vx[0];
  V res(p.dim());
  res[0] = 1;
  res[1] = x;
  res[2] = x*x;
  res[3] = x*x*x;
  return res;
}

V j_auto(const Params &p, const V &vx){
  const real x = vx[0];
  V res(p.dim());
  real delta = 0.000001;
  for(int i=0;i<p.dim();++i){
    V pPlus = p, pMinus = p;
    pPlus[i] +=delta;
    pMinus[i] -=delta;
    res[i] = ( f(pPlus,vx) - f(pMinus,vx) ) / delta;
  }
  return res;
}


std::vector<XY> create_data(){
  URand r(-4,4);
  const int N = 100;
  XY init = { V(1), 0 };
  std::vector<XY> data(N,init);
  for(int i=0;i<N;++i){
    real x = r;
    data[i].x[0] = x;
    data[i].y = 1 + 2*x + 3*x*x + 4*x*x*x; 
  }
  return data;
}

int main(){
  Params initParams(4);
  std::fill(initParams.begin(),initParams.end(),0.1);
  Params opt = lma(create_data(),f,j_auto,initParams);
  SHOW(opt);
}
