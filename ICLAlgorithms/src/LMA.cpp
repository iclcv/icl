/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLUtils/src/LM.cpp                                    **
 ** Module : ICLUtils                                               **
 ** Authors: Christian Groszewski                                   **
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
#include <ICLAlgorithms/LMA.h>
#include <ICLUtils/StringUtils.h>

namespace icl{

LMA::LMA(){}

LMA::~LMA(){}

double LMA::dot(DynMatrix<icl64f> &a, DynMatrix<icl64f> &b){
	double dotp = 0.0;
	if(a.rows() == b.rows()){
		for(unsigned int i=0;i<a.rows();++i){
			dotp += (a[i]*b[i]);
		}
	}
	return dotp;
}

void LMA::gradient_descent(const DynMatrix<icl64f> &x,DynMatrix<icl64f> &params,
		DynMatrix<icl64f> *(*func)(const DynMatrix<icl64f> &x, DynMatrix<icl64f> &params),
		DynMatrix<icl64f> *(*jacobian)(const DynMatrix<icl64f> &x, DynMatrix<icl64f> &params),
		double error, int MaxIter){

	double change = 1.0;
	int iter = 0;
	DynMatrix<icl64f> *jac = 0;
	DynMatrix<icl64f> *val = 0;
	DynMatrix<icl64f> ex(1,x.rows());
	double err = 0;
	while ((change > 1e-10)&&(iter < MaxIter)){
		//std::cout << "iter: " << iter << "  change: " << change << std::endl;
		jac = jacobian(x, params);
		val = func(x,params);
		//err = 0;
		for(unsigned int i=0;i<x.rows();++i){
			ex[i] = -(*val)[i];
			err += (*val)[i]*(*val)[i];

		}
		delete val;
		err = std::sqrt(err);

		bool stop = false;
		std::string aa = str(ex);
		for(unsigned int i=0;i<jac->dim();++i){
			int m = isnan((*jac)[i]);
			int n = isinf((*jac)[i]);
			if(m !=0 || n!=0)
				stop = true;
		}
		if (stop){//jac->cond() > thresh_cond || k){
			std::cout << "ohoh\n";
			change = 0;
			//delete jac;
		} else {

			DynMatrix<icl64f> JJT = jac->transp();
			DynMatrix<icl64f> JJ2 = JJT*(*jac);
			if(JJ2.det() <1e-8){
				change = 0;
				delete jac;
				continue;
			}

			DynMatrix<icl64f> param_innov(1,params.dim()), param_up(1,params.dim());
			DynMatrix<icl64f> temp = JJ2.pinv()*JJT;
			param_innov = temp*ex;

			param_up = params + param_innov;
			change = param_innov.norm()/param_up.norm();
			params = param_up;
			//SHOW(params);
			iter = iter + 1;
		}
		delete jac;
	}
}

//ok
double LMA::solve(const DynMatrix<icl64f> &x,
		DynMatrix<icl64f> *(*func)(const DynMatrix<icl64f> &x, DynMatrix<icl64f> &params),
		DynMatrix<icl64f> *(*jacobian)(const DynMatrix<icl64f> &x, DynMatrix<icl64f> &params),
		DynMatrix<icl64f> &params_est,const double thresh, const int n_iters, bool debug_output){

	double lambda=0.1;
	bool updateJ=true;
	int paramcount = params_est.rows();
	DynMatrix<icl64f> *y_est = 0;
	DynMatrix<icl64f> *y_est_lm = 0;
	DynMatrix<icl64f> d(1,x.rows());
	DynMatrix<icl64f> dp(1,x.rows());
	DynMatrix<icl64f> d_lm(1,x.rows());
	DynMatrix<icl64f> *J = 0;
	DynMatrix<icl64f> Jt;
	DynMatrix<icl64f> H(paramcount,paramcount);
	DynMatrix<icl64f> H_lm;
	DynMatrix<icl64f> eye(paramcount,paramcount);

	DynMatrix<icl64f> params_lm(1,params_est.rows());
	double e = 10e6;
	double e_lm = 0.0;
	int it=0;
	while(it<n_iters && thresh < e){
		if(debug_output)
			std::cout << "iter: " << it << std::endl;
		if(updateJ){
			if(J)
				delete J;
			J = jacobian(x, params_est);
			if(y_est)
				delete y_est;
			y_est = func(x,params_est);
			for(unsigned int i=0;i<y_est->rows();++i)
				d[i] = (*y_est)[i];
			Jt = J->transp();
			(Jt).mult(*J,H);
			if (it==0){
				e=dot(d,d);
			}
		}
		for(int i=0;i<paramcount;++i){
			eye.at(i,i) = lambda;
		}

		DynMatrix<icl64f> dst;
		Jt.mult(d,dst);
		H_lm = H + eye;

		dp = H_lm.solve(dst);
		dp.mult(-1,dp);
		DynMatrix<icl64f> dst2;

		for(int i=0;i<paramcount;++i){
			params_lm[i] = params_est[i]+dp[i];
		}

		if(y_est_lm)
			delete y_est_lm;
		y_est_lm = func(x,params_lm);
		for(unsigned int i=0;i<y_est->rows();++i)
			d_lm[i]=(*y_est_lm)[i];
		e_lm=dot(d_lm,d_lm);
		if(debug_output)
			std::cout << e << std::endl;
		if (e_lm<e){
			lambda=lambda/10.0;
			for(int i=0;i<paramcount;++i){
				params_est[i] = params_lm[i];
			}
			e=e_lm;
			updateJ=true;
			if(e<thresh)
				break;
		} else {
			updateJ=false;
			lambda=lambda*10;
		}
		++it;
	}
	if(J)
		delete J;
	if(y_est_lm)
		delete y_est_lm;
	if(y_est)
		delete y_est;
	return e;
}

}
