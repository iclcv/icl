/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/examples/levenberg-marquardt-example.cpp       **
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

#include <ICLMath/LevenbergMarquardtFitter.h>
#include <ICLUtils/Random.h>

using namespace icl::utils;
using namespace icl::math;

typedef float real;
typedef LevenbergMarquardtFitter<real> LM;
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
  dst[2] = y*z*d*d + k;
  dst[3] = 2*d*y*z*c + k;
  dst[4] = -z*x*f*b;
  dst[5] = -z*x*e*b;
}

void cmp3(LMFunction f,  Params a, Params b){
  LM::Data d = LM::create_data(a,f,3);
  for(unsigned int i=0;i<d.x.rows();++i){
    real y = f(b,Vector(3,d.x.row_begin(i)));
    std::cout << " d(" << i << "):" << fabs(y - d.y[i]) << std::endl;
  }
}


void dbg(const LM::Result &d){
  std::cout << d << std::endl;
}

int main(int n, char **ppc){
  const real v[] = {1,2,3,4};
  Params p(4,v);
  LM::Data d = LM::create_data(p,f,1);
  Result result;

  // f
  LM lm(f);
  //  lm.setDebugCallback(dbg);


  std::cout << "fitting f(x) = a + b*x + c*x^2 + d*x^3 ((a,b,c,d)=(1,2,3,4), using numeric jacobian)" << std::endl;
  result = lm.fit(d.x,d.y,Params(4,1));
  std::cout << "final parameters: " << result << std::endl << std::endl; 

  std::cout << "fitting f(x) = a + b*x + c*x^2 + d*x^3 ((a,b,c,d)=(1,2,3,4), using analytic jacobian)" << std::endl;
  lm.init(f,j);
  result = lm.fit(d.x,d.y,Params(4,1));
  std::cout << "final parameters: " << result << std::endl << std::endl; 


  // f2
  std::cout << "fitting f(x) = a + a*a*x + b*b*x + a*b*x + c*x^2 ((a,b,c) = (1,2,3) using numeric jacobian)" << std::endl;
  lm.init(f2);
  d = LM::create_data(p,f2,1);
  result = lm.fit(d.x,d.y,Params(3,0.1));
  std::cout << "final parameters: " << result << std::endl << std::endl; 

  std::cout << "fitting f(x) = a + a*a*x + b*b*x + a*b*x + c*x^2 ((a,b,c) = (1,2,3) using analytic jacobian)" << std::endl;
  lm.init(f2,j2);
  result = lm.fit(d.x,d.y,Params(3,0.1));
  std::cout << "final parameters: " << result << std::endl << std::endl; 

  std::cout << "fitting f(x) = a + a*a*x + b*b*x + a*b*x + c*x^2 ((a,b,c) = "
            << "(1,2,3) using analytic jacobian + hessian)" << std::endl;
  lm.init(f2,j2);
  result = lm.fit(d.x,d.y,Params(3,0.1));
  std::cout << "final parameters: " << result << std::endl << std::endl; 

  // f3
  std::cout << "fitting f(x,y,z) = x*y*a*b + y*z*d*d*c - z*x*f*e*b + sqr(a + b + c + d) (using numeric jacobian)" << std::endl;
  const real v2[] = {1,2,3,4,5,6};
  p = Params(6,v2);
  d = LM::create_data(p,f3,3);
  lm.init(f3);
  result = lm.fit(d.x,d.y,Params(6,4));
  std::cout << "final parameters: " << result << std::endl << std::endl; 

  std::cout << "fitting f(x,y,z) = x*y*a*b + y*z*d*d*c - z*x*f*e*b + sqr(a + b + c + d) (using analytic jacobian)" << std::endl;
  lm.init(f3,j3);
  result = lm.fit(d.x,d.y,Params(6,4));
  std::cout << "final parameters: " << result << std::endl << std::endl; 

  
  
  //cmp3(f3,p,result.params);

  

}

