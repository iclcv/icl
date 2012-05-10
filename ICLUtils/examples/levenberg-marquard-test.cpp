#include <ICLUtils/LevenbergMarquardFitter.h>
#include <ICLUtils/Random.h>

using namespace icl;

typedef LevenbergMarquardFitter<float>::Function LMFunction;
typedef LevenbergMarquardFitter<float>::Matrix Matrix;
typedef LevenbergMarquardFitter<float>::Vector Vector;
typedef LevenbergMarquardFitter<float>::Params Params;
typedef LevenbergMarquardFitter<float>::Result Result;

float f(const Params &p, const Vector &vx){
  const float x = vx[0];
  return p[0] + p[1]*x + p[2]*x*x + p[3] * x*x*x;
}

void j(const Params &p, const Vector &vx, Vector &dst){
  const float x = vx[0];
  dst[0] = 1;
  dst[1] = x;
  dst[2] = x*x;
  dst[3] = x*x*x;
  
}


std::pair<Matrix,Vector> data(const Params &p, LMFunction f){
  URand r(-4,4);
  const int N = 1000;
  Matrix x(1,N);
  Vector y(N);
  for(int i=0;i<N;++i){
    x[i] = r;
    y[i] = f(p,x.row(i));
  }
  return std::pair<Matrix,Vector>(x,y);
}

void dbg(const LevenbergMarquardFitter<float>::Result &d){
  std::cout << d << std::endl;
}

int main(int n, char **ppc){
  const float v[] = {1,2,3,4};
  Params p(4,v);
  std::pair<Matrix,Vector> d = data(p,f);


  LevenbergMarquardFitter<float> lm(f);
  lm.setDebugCallback(dbg);
  std::cout << "fitting f(x) = 1 + 2*x + 3*x^2 + 4*x^3 (using approximated jacobian)" << std::endl;
  Result result = lm.fit(d.first,d.second,Params(4,1));
  std::cout << "final parameters: " << result << std::endl << std::endl; 

  std::cout << "fitting f(x) = 1 + 2*x + 3*x^2 + 4*x^3 (using real jacobian)" << std::endl;
  lm.init(f,j);
  result = lm.fit(d.first,d.second,Params(4,1));
  std::cout << "final parameters: " << result << std::endl << std::endl; 

}

