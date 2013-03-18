/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/examples/levenberg-marquardt.cpp               **
** Module : ICLMath                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
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
#include <ICLUtils/ProgArg.h>
#include <ICLMath/FixedVector.h>

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

LM::Vector f2(const LM::Params &p, const LM::Vector &vx){
  const real x = vx[0];
  return LM::Vector(1,p[0] + sqr(p[0])*x + sqr(p[1])*x + p[0]*p[1]*x + p[2]*x*x);
}

void j2(const LM::Params &p, const LM::Vector &vx, LM::Vector &dst){
  real x = vx[0];
  real a = p[0], b = p[1];
  dst[0] = 1 + 2*a*x + b*x;
  dst[1] = 2*b*x + a*x;
  dst[2] = x*x;
}

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

LM::Vector f4(const LM::Params &p, const LM::Vector &vx){
  FixedColVector<real,4> x(vx[0],vx[1], vx[2], 1);
  FixedMatrix<real,4,4> T = create_hom_4x4<float>(p[0],p[1],p[2],p[3],p[4],p[5]);
  FixedColVector<real,4> y = T*x;
  return LM::Vector(3,y.begin());
}


void dbg(const LM::Result &res){
  std::cout << res << std::endl;
}


#if 1
int main(){
  LM lm(f4,3);
  const real r[6] = { 3, 1, 2, 100, 100, 100};
  const real s[6] = { 0.1, 0.1, 0.1, 10, 10, 10 };
  LM::Params realParams(6,r), startParams(6,s);
  LM::Data data = LM::create_data(realParams,f4,3,3);
  LM::Result result = lm.fit(data.x,data.y,startParams);
  SHOW(result);
}
#else
int main(int n, char **ppc){
  pa_explain("-f","selection function to optimize:\n"
             "0: f(x) = a + b*x + c*x^2 + d*x^3\n"
             "1: f(x) = a + a*a*x + b*b*x + a*b*x + c*x^2\n"
             "2: f(x,y,z) = x*y*a*b + y*z*d*d*c - z*x*f*e*b + sqr(a + b + c + d)"
             "3: f(x,y,z) = rot(a,b,c)*(x,y,z)^T + (d,e,f)^T");

  pa_init(n,ppc,"-function-index|-f(int) -use-analytic-jacobian|-aj -enable-debug-callback|-d");
  LM::Function fs[4] = {f,f2,f3,f4};
  LM::Jacobian js[4] = { j, j2, j3, LM::Jacobian()};
  int numParams[4] = { 4, 3, 6, 6};
  int numInputs[4] = { 1, 1, 1, 3};
  int numOutputs[4] = { 1, 1, 1, 3};
  
  bool useAnalyticJacobian = pa("-aj");
  int idx = pa("-f");
  if(idx < 0 || idx > 3) throw ICLException("-f idx: idx must be in range [0,3]");


  LM *lm = ( useAnalyticJacobian ? 
             new LM(fs[idx],numOutputs[idx],std::vector<LM::Jacobian>(idx!=3,js[idx])) :
             new LM(fs[idx],numOutputs[idx]) );
  
  if(pa("-d")) lm->setDebugCallback(dbg);

  LM::Params realParams(numParams[idx]);
  LM::Params startParams(numParams[idx],1);
  if(idx == 3){
    real r[6] = {3, 1, 2, 100, 100, 100};
    real s[6] = { 0.1, 0.1, 0.1, 10, 10, 10 };
    std::copy(r,r+6, realParams.begin());
    std::copy(s,s+6, startParams.begin());
  }else{
    for(int i=0;i<numParams[idx];++i){
      realParams[i] = i+1;
    }
  }


  LM::Data data = LM::create_data(realParams,fs[idx],numInputs[idx],numOutputs[idx]);
  LM::Result result = lm->fit(data.x,data.y,startParams);
  SHOW(result);
}

#endif
