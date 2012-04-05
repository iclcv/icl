/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLGeom/examples/camcalib-demo.cpp                     **
 ** Module : ICLGeom                                                **
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
using namespace icl;

double *y = 0;
double *x = 0;

/*the function to be fit a*cos(b*x)+b*sin(a*x)
*/
double func(double x, double *params){
	return params[0]*std::cos(params[1]*x)+params[1]*std::sin(params[0]*x);
}

/* definition of errorfunction to be optimized
 * a*cos(b*x)+b*sin(a*x)-y
 * where a = params[0] an b=params[1] are the parameters to be optimized
 */
DynMatrix<icl64f> *func2(const DynMatrix<icl64f> &x, DynMatrix<icl64f> &params){
	DynMatrix<icl64f> *res = new DynMatrix<icl64f>(1,x.rows());
	for(unsigned int i=0;i<60;++i)
		res->at(0,i) = params[0]*std::cos(params[1]*x[i])+params[1]*std::sin(params[0]*x[i])-y[i];
	return res;
}

//jacobian matrix for a*cos(b*x)+b*sin(a*x)-y  with respect to a and b
DynMatrix<icl64f> *jacobian2(const DynMatrix<icl64f> &x, DynMatrix<icl64f> &params){
	DynMatrix<icl64f> *res = new DynMatrix<icl64f>(2,60);
	for(unsigned int i=0;i<x.rows();++i){
		res->at(0,i) = (std::cos(params[1]*x[i])+params[1]*std::cos(params[0]*x[i])*x[i]);
		res->at(1,i) = -params[0]*std::sin(params[1]*x[i])*x[i]+std::sin(params[0]*x[i]);
	}
	return res;
}

//forward differences approximating jacobian for a*cos(b*x)+b*sin(a*x)-y with respect to a and b
DynMatrix<icl64f>* jacobian3(const DynMatrix<icl64f> &x, DynMatrix<icl64f> &params){
	DynMatrix<icl64f> *mat = new DynMatrix<icl64f>(params.dim(),x.rows());
	DynMatrix<icl64f> *f = func2(x,params);
	DynMatrix<icl64f> temp(params.cols(),params.rows());
	double hh = 0.25*1e-6;
	double hhh = 1E-04;
	double hinv=1/hh;
	for(unsigned int j=0;j<params.dim();++j){
		for(unsigned int m=0;m<params.dim();++m)
			temp[m] = params[m];
		hh = hhh*temp[j];
		hinv = 1/hh;
		temp[j]=temp[j]+(hh);
		DynMatrix<icl64f> *fh=func2(x,temp);

		DynMatrix<icl64f> part = ((*fh)-(*f));
		for(unsigned int k=0;k<x.rows();++k){
			mat->at(j,k)=part[k]*hinv;
		}
		delete fh;
	}
	delete f;
	return mat;
}

void init_lma(){
	std::cout << "optimizing function a*cos(b*x)+b*sin(a*x)" << std::endl;
	srand(time(0)+getpid());
	y = new double[60];
	x = new double[60];
	//initial parameters
	double pp[2] = {100.0, 102.0};
	std::cout << "init params for data: a=" << pp[0] << "  b=" << pp[1] << std::endl;
	double xx = 0.0;
	int counter = 0;
	while(xx < 6 && counter < 60){
		x[counter] = xx;
		y[counter] = func(xx, pp);//+(5.0*rand())/RAND_MAX;
		++counter;
		xx += 0.1;
	}
}

int main(int n, char **args){
	init_lma();
	LMA lma;
	double p[2] = {100.5, 102.5};
	std::cout << "first guess for params: a=" << p[0] << "  b=" << p[1] << std::endl;
	DynMatrix<icl64f> xx(1,60,x);
	DynMatrix<icl64f> pp(1,2,p);
	//lma.solve(xx,func2,jacobian3,pp,0.000000025,100);
	lma.solve(xx,func2,jacobian2,pp,0.000000025,1000);
	//lma.gradient_descent(xx,pp,func2,jacobian3,0.00025,100);
	SHOW(pp);
	delete[] x;
	delete[] y;
	return 0;
}
