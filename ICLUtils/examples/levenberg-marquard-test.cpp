#include <ICLUtils/LevenbergMarquardFitter.h>
#include <ICLUtils/Random.h>

using namespace icl;
typedef float real;
typedef LevenbergMarquardFitter<real> LM;
typedef LM::Function LMFunction;
typedef LM::Jacobian Jacobian;
typedef LM::Matrix Matrix;
typedef LM::Vector Vector;
typedef LM::Params Params;
typedef LM::Result Result;

real f(const Params &p, const Vector &vx){
  const real x = vx[0];
  return p[0] + p[1]*x + p[2]*x*x + p[3] * x*x*x;
}

void j(const Params &p, const Vector &vx, Vector &dst){
  const real x = vx[0];
  dst[0] = 1;
  dst[1] = x;
  dst[2] = x*x;
  dst[3] = x*x*x;
  
}

real f2(const Params &p, const Vector &vx){
  const real x = vx[0];
  return p[0] + sqr(p[0])*x + sqr(p[1])*x + p[0]*p[1]*x + p[2]*x*x;
}

void j2(const Params &p, const Vector &vx, Vector &dst){
  real x = vx[0];
  real a = p[0], b = p[1];
  dst[0] = 1 + 2*a*x + b*x;
  dst[1] = 2*b*x + a*x;
  dst[2] = x*x;
}
std::pair<Matrix,Vector> data(const Params &p, LMFunction f){
  URand r(-4,4);
  const int N = 100;
  Matrix x(1,N);
  Vector y(N);
  for(int i=0;i<N;++i){
    x[i] = r;
    y[i] = f(p,x.row(i));
  }
  return std::pair<Matrix,Vector>(x,y);
}

std::pair<Matrix,Vector> data3D(const Params &p, LMFunction f){
  const int N = 1000;
  Matrix x(3,N);
  std::fill(x.begin(),x.end(),URand(-5,5));
  Vector y(N);
  for(int i=0;i<N;++i){
    y[i] = f(p,Vector(3,x.row_begin(i),false));
  }
  return std::pair<Matrix,Vector>(x,y);
}

real f3(const Params &p, const Vector &vx){
  real x = vx[0], y=vx[1], z=vx[2];
  real a = p[0], b=p[1], c=p[2], d=p[3], e=p[4], f=p[5];
  return x*y*a*b + y*z*d*d*c - z*x*f*e*b + sqr(a + b + c + d);
}

void j3(const Params &p, const Vector &vx, Vector &dst){
  real x = vx[0], y=vx[1], z=vx[2];
  real a = p[0], b=p[1], c=p[2], d=p[3], e=p[4], f=p[5];
  real k = 2 * (a+b+c+d);
  dst[0] = x*y*b + k;
  dst[1] = x*y*a  - z*x*f*e + k;
  dst[2] = y*z*d*d  - z*x*f*e + k;
  dst[3] = 2*d*y*z*c + k;
  dst[4] = -z*x*f*b;
  dst[5] = -z*x*e*b;
}

void cmp3(LMFunction f,  Params a, Params b){
  std::pair<Matrix,Vector> d = data3D(a,f);
  for(unsigned int i=0;i<d.first.rows();++i){
    real y = f(b,Vector(3,d.first.row_begin(i)));
    std::cout << " d(" << i << "):" << fabs(y - d.second[i]) << std::endl;
  }
}


void dbg(const LM::Result &d){
  std::cout << d << std::endl;
}

int main(int n, char **ppc){
  const real v[] = {1,2,3,4};
  Params p(4,v);
  std::pair<Matrix,Vector> d = data(p,f);
  Result result;

  // f
  LM lm(f);
  lm.setDebugCallback(dbg);


  std::cout << "fitting f(x) = a + b*x + c*x^2 + d*x^3 ((a,b,c,d)=(1,2,3,4), using numeric jacobian)" << std::endl;
  result = lm.fit(d.first,d.second,Params(4,1));
  std::cout << "final parameters: " << result << std::endl << std::endl; 

  std::cout << "fitting f(x) = a + b*x + c*x^2 + d*x^3 ((a,b,c,d)=(1,2,3,4), using analytic jacobian)" << std::endl;
  lm.init(f,j);
  result = lm.fit(d.first,d.second,Params(4,1));
  std::cout << "final parameters: " << result << std::endl << std::endl; 


  // f2
  std::cout << "fitting f(x) = a + a*a*x + b*b*x + a*b*x + c*x^2 ((a,b,c) = (1,2,3) using numeric jacobian)" << std::endl;
  lm.init(f2);
  d = data(p,f2);
  result = lm.fit(d.first,d.second,Params(3,0.1));
  std::cout << "final parameters: " << result << std::endl << std::endl; 

  std::cout << "fitting f(x) = a + a*a*x + b*b*x + a*b*x + c*x^2 ((a,b,c) = (1,2,3) using analytic jacobian)" << std::endl;
  lm.init(f2,j2);
  result = lm.fit(d.first,d.second,Params(3,0.1));
  std::cout << "final parameters: " << result << std::endl << std::endl; 


  // f3
  std::cout << "fitting f(x,y,z) = x*y*a*b + y*z*d*d*c - z*x*f*e*b + sqr(a + b + c + d) (using numeric jacobian)" << std::endl;
  const real v2[] = {1,2,3,4,5,6};
  p = Params(6,v2);
  d = data3D(p,f3);
  lm.init(f3);
  result = lm.fit(d.first,d.second,Params(6,4));
  std::cout << "final parameters: " << result << std::endl << std::endl; 

  std::cout << "fitting f(x,y,z) = x*y*a*b + y*z*d*d*c - z*x*f*e*b + sqr(a + b + c + d) (using analytic jacobian)" << std::endl;
  lm.init(f3,j3);
  result = lm.fit(d.first,d.second,Params(6,4));
  std::cout << "final parameters: " << result << std::endl << std::endl; 

  
  
  //cmp3(f3,p,result.params);

  

}

