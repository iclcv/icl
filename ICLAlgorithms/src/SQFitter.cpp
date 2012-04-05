/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLAlgorithms/src/SQFitter.cpp                         **
 ** Module : ICLAlgorithms                                          **
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
#include <ICLAlgorithms/SQFitter.h>

namespace icl{

SuperQuadric SQFitter::sq;

struct SQFitter::Data{

	const DynMatrix<icl64f> *dat;
	double scale;

	Data(const DynMatrix<icl64f> *data):dat(data),scale(0.0){}

	~Data(){
	}
};

SQFitter::~SQFitter(){
	delete m_data;
}

SQFitter::SQFitter(const DynMatrix<icl64f> *data):m_data(new SQFitter::Data(data)){}

DynMatrix<icl64f> SQFitter::euler_from_rot_matrix(DynMatrix<icl64f> &R){
	// extracts the euler angles rx, ry, rz from a rotation matrix and returns them
	//   taken from http://www.flipcode.com/documents/matrfaq.html#Q37 """
	double rx = 0;
	double ry = -asin(R(2,0));
	double rz = 0;
	double c = cos(ry);
	if (fabs(c) > 1.0e-5){ //# no gimbal lock?
		rx = atan2(-R(2,1)/c, R(2,2)/c);
		rz = atan2(-R(1,0)/c, R(0,0)/c);
	}else{
		rx = 0;
		rz = atan2(R(0,1), R(1,1));
	}
	DynMatrix<icl64f> res(1,3);
	res[0] = rx; res[1] = ry; res[2] = rz;
	return res;
}

DynMatrix<icl64f> SQFitter::sort_eig(DynMatrix<icl64f> &w, DynMatrix<icl64f> &R){
	w[0] = fabs(w[0]); w[1] = fabs(w[1]); w[2] = fabs(w[2]);
	DynMatrix<icl64f> newR(3,3);
	if (w[0] >= w[1] && w[0] >= w[2]){ //# w0 biggest?
		if (w[1] >= w[2]){
			//return np.array([R[:,2], R[:,1], R[:,0]]).transpose();
			newR(0,0) = R(2,0); newR(1,0) = R(1,0); newR(2,0) = R(0,0);
			newR(0,1) = R(2,1); newR(1,1) = R(1,1); newR(2,1) = R(0,1);
			newR(0,2) = R(2,2); newR(1,2) = R(1,2); newR(2,2) = R(0,2);
		}else{
			//return np.array([R[:,1], R[:,2], R[:,0]]).transpose();
			newR(0,0) = R(1,0); newR(1,0) = R(2,0); newR(2,0) = R(0,0);
			newR(0,1) = R(1,1); newR(1,1) = R(2,1); newR(2,1) = R(0,1);
			newR(0,2) = R(1,2); newR(1,2) = R(2,2); newR(2,2) = R(0,2);
		}
	} else if (w[1] >= w[0] && w[1] >= w[2]){ //# w1 biggest?
		if (w[0] >= w[2]){
			//return np.array([R[:,2], R[:,0], R[:,1]]).transpose();
			newR(0,0) = R(2,0); newR(1,0) = R(0,0); newR(2,0) = R(1,0);
			newR(0,1) = R(2,1); newR(1,1) = R(0,1); newR(2,1) = R(1,1);
			newR(0,2) = R(2,2); newR(1,2) = R(0,2); newR(2,2) = R(1,2);
		} else{
			//return np.array([R[:,0], R[:,2], R[:,1]]).transpose();
			newR(0,0) = R(0,0); newR(1,0) = R(2,0); newR(2,0) = R(1,0);
			newR(0,1) = R(0,1); newR(1,1) = R(2,1); newR(2,1) = R(1,1);
			newR(0,2) = R(0,2); newR(1,2) = R(2,2); newR(2,2) = R(1,2);
		}
	} else{ //# w2 biggest!
		if (w[0] >= w[1]){
			//return np.array([R[:,1], R[:,0], R[:,2]]).transpose();
			newR(0,0) = R(1,0); newR(1,0) = R(0,0); newR(2,0) = R(2,0);
			newR(0,1) = R(1,1); newR(1,1) = R(0,1); newR(2,1) = R(2,1);
			newR(0,2) = R(1,2); newR(1,2) = R(0,2); newR(2,2) = R(2,2);
		} else{
			//return R
			newR.set_data(R.data());
		}
	}
	newR = newR.transp();
	return newR;
}

double SQFitter::mean(const DynMatrix<icl64f> &m){
	double mean = 0.0;
	for(unsigned int i=0;i<m.dim();++i){
		mean += m[i];
	}
	return mean/m.dim();
}

double SQFitter::min(const DynMatrix<icl64f> &m){
	double min = m[0];
	for(unsigned int i=1; i<m.dim();++i){
		if(m[i]<min)
			min = m[i];
	}
	return min;
}

double SQFitter::max(const DynMatrix<icl64f> &m){
	double max = m[0];
	for(unsigned int i=1; i<m.dim();++i){
		if(m[i]>max)
			max = m[i];
	}
	return max;
}

DynMatrix<icl64f> SQFitter::initial_estimate(const DynMatrix<icl64f> &x, const DynMatrix<icl64f> &y, const DynMatrix<icl64f> &z){
	// makes an initial estimation of sq model parameters.
	SQFitter::sq.sete1e2(1.0,1.0);

	DynMatrix<icl64f> M(3,3);
	M[0] = mean(y.elementwise_mult(y) + z.elementwise_mult(z));
	M[1] = -mean(y.elementwise_mult(x));
	M[2] = -mean(z.elementwise_mult(x));
	M[3] = -mean(y.elementwise_mult(x));
	M[4] = mean(x.elementwise_mult(x) + z.elementwise_mult(z));
	M[5] = -mean(z.elementwise_mult(y));
	M[6] = -mean(x.elementwise_mult(z));
	M[7] = -mean(y.elementwise_mult(z));
	M[8] = -mean(x.elementwise_mult(x) + y.elementwise_mult(y));
	//# find rotation matrix that makes M diagonal:
	DynMatrix<icl64f> R(3,3);
	DynMatrix<icl64f> w(1,3);
	M.eigen(R,w);
	//DynMatrix<icl64f> newR = &R;
	DynMatrix<icl64f> newR = sort_eig(w,R);
	DynMatrix<icl64f> rp = euler_from_rot_matrix(newR);
	SQFitter::sq.updateR(rp);
	//SQFitter::sq.setrxryrz(rp[0],rp[1],rp[2]);

	DynMatrix<icl64f> vec(x.rows(),3);
	for(unsigned int i=0;i<x.rows();++i){
		vec(i,0) = x[i]; vec(i,1) = y[i]; vec(i,2) = z[i];
	}
	DynMatrix<icl64f> Rinv = newR.inv();

	DynMatrix<icl64f> vec2 = Rinv*vec;

	DynMatrix<icl64f> xt(1,x.rows());
	DynMatrix<icl64f> yt(1,x.rows());
	DynMatrix<icl64f> zt(1,x.rows());
	for(unsigned int i=0;i<x.rows();++i){
		xt[i] = vec2(i,0); yt[i] = vec2(i,1); zt[i] = vec2(i,2);
	}

	// set dimensions
	double maxx = max(xt);
	double minx = min(xt);
	double maxy = max(yt);
	double miny = min(yt);
	double maxz = max(zt);
	double minz = min(zt);
	double dimx = (maxx-minx);
	double dimy = (maxy-miny);
	double dimz = (maxz-minz);
	SQFitter::sq.seta1a2a3(dimx/2.0,dimy/2.0,dimz/2.0);
	// set center point
	DynMatrix<icl64f> center(1,3);
	center[0] = (maxx+minx)/2.0; center[1] = (maxy+miny)/2.0; center[2] = (maxz+minz)/2.0;
	center = newR*center;

	//SQFitter::sq.setXYZ(center[0],center[1],center[2]);
	SQFitter::sq.updateXYZ(center);
	DynMatrix<icl64f> dims(1,3); dims[0] = dimx; dims[1] = dimy; dims[2] = dimz;
	return dims;
}

void SQFitter::fit(DynMatrix<icl64f> &params, const double thresh, const int maxiter, bool autoinit){
	unsigned int rows = (m_data->dat)->rows();
	DynMatrix<icl64f> x(1,rows);
	DynMatrix<icl64f> y(1,rows);
	DynMatrix<icl64f> z(1,rows);
	for(unsigned int i=0;i<rows;++i){
		x[i] = (m_data->dat)->at(0,i);
		y[i] = (m_data->dat)->at(1,i);
		z[i] = (m_data->dat)->at(2,i);
	}
	// move to zero and scale
	double dx = mean(x);
	double dy = mean(y);
	double dz = mean(z);
	m_data->scale = 1/pow(( pow((max(x)-min(x)),2.0) + pow((max(y)-min(y)),2.0) + pow((max(z)-min(z)),2.0) ),0.5);

	for(unsigned int i=0;i<rows;++i){
		x[i] = (x[i]-dx)*m_data->scale;
		y[i] = (y[i]-dy)*m_data->scale;
		z[i] = (z[i]-dz)*m_data->scale;
	}

	if(autoinit)
		DynMatrix<icl64f> dims = initial_estimate(x,y,z);

	SQFitter::sq.updateAll(params);

	DynMatrix<icl64f> newxyz2(3,rows);
	for(unsigned int i=0;i<x.rows();++i){
		newxyz2(0,i) = x[i]; newxyz2(1,i) = y[i]; newxyz2(2,i) = z[i];
	}

	LMA lma;
	lma.solve(newxyz2,func_real,jacobian,params,thresh,maxiter);

	//of course this method does not (very well)
	//lma.gradient_descent(newxyz2,params_est,func_real,jacobian,thresh,maxiter);

	//undo scaling
	m_data->scale = 1.0/m_data->scale;

	params[5] = params[5]*m_data->scale;
	params[6] = params[6]*m_data->scale;
	params[7] = params[7]*m_data->scale;

	params[0] = params[0]*m_data->scale+dx;
	params[1] = params[1]*m_data->scale+dy;
	params[2] = params[2]*m_data->scale+dz;
}

DynMatrix<icl64f> *SQFitter::func_real(const DynMatrix<icl64f> &xyz, DynMatrix<icl64f> &params){
	SQFitter::sq.updateAll(params);

	double *data = SQFitter::sq.getAllParams();
	for(unsigned int i=0;i<params.dim();++i){
		params[i] = data[i];
	}
	delete[] data;
	DynMatrix<icl64f> x(1,xyz.rows());
	DynMatrix<icl64f> y(1,xyz.rows());
	DynMatrix<icl64f> z(1,xyz.rows());
	for(unsigned int i=0;i<xyz.rows();++i){
		x[i] = xyz(0,i);
		y[i] = xyz(1,i);
		z[i] = xyz(2,i);
	}
	DynMatrix<icl64f> *mat = SQFitter::sq.error(x,y,z);
	return mat;
}

//forwarddifferences
DynMatrix<icl64f>* SQFitter::jacobian(const DynMatrix<icl64f> &xyz, DynMatrix<icl64f> &params){
	DynMatrix<icl64f> *mat = new DynMatrix<icl64f>(params.dim(),xyz.rows());
	DynMatrix<icl64f> *f = func_real(xyz,params);
	DynMatrix<icl64f> temp(params.cols(),params.rows());
	double hh = 0.25*1e-6;
	double hhh = 1E-04;
	double hinv=1/hh;
	for(unsigned int j=0;j<params.dim();++j){
		for(unsigned int m=0;m<params.dim();++m)
			temp[m] = params[m];
		hh = hhh*temp[j];
		if(hh == 0)
			hh = 0.125*1e-6;
		hinv = 1/hh;
		temp[j]=temp[j]+(hh);
		DynMatrix<icl64f> *fh=func_real(xyz,temp);

		DynMatrix<icl64f> part = ((*fh)-(*f));
		for(unsigned int k=0;k<xyz.rows();++k){
			mat->at(j,k)=part[k]*hinv;
		}
		delete fh;
	}
	delete f;
	return mat;
}

DynMatrix<icl64f> *SQFitter::func_XYZ(const DynMatrix<icl64f> &xyz, DynMatrix<icl64f> &params){
	SQFitter::sq.updateXYZ(params);
	//this is for clipping
	double *data = SQFitter::sq.getXYZ();
	for(unsigned int i=0;i<params.dim();++i){
		params[i] = data[i];
	}
	delete[] data;
	DynMatrix<icl64f> x(1,xyz.rows());
	DynMatrix<icl64f> y(1,xyz.rows());
	DynMatrix<icl64f> z(1,xyz.rows());
	for(unsigned int i=0;i<xyz.rows();++i){
		x[i] = xyz(0,i);
		y[i] = xyz(1,i);
		z[i] = xyz(2,i);
	}
	DynMatrix<icl64f> *mat = SQFitter::sq.error(x,y,z);
	return mat;
}

DynMatrix<icl64f> *SQFitter::func_R(const DynMatrix<icl64f> &xyz, DynMatrix<icl64f> &params){
	SQFitter::sq.updateR(params);
	//this is for clipping
	double *data = SQFitter::sq.getR();
	for(unsigned int i=0;i<params.dim();++i){
		params[i] = data[i];
	}
	delete[] data;
	DynMatrix<icl64f> x(1,xyz.rows());
	DynMatrix<icl64f> y(1,xyz.rows());
	DynMatrix<icl64f> z(1,xyz.rows());
	for(unsigned int i=0;i<xyz.rows();++i){
		x[i] = xyz(0,i);
		y[i] = xyz(1,i);
		z[i] = xyz(2,i);
	}
	DynMatrix<icl64f> *mat = SQFitter::sq.error(x,y,z);
	return mat;
}

DynMatrix<icl64f> *SQFitter::func_E(const DynMatrix<icl64f> &xyz, DynMatrix<icl64f> &params){
	SQFitter::sq.updateE(params);
	//this is for clipping
	double *data = SQFitter::sq.getE();
	for(unsigned int i=0;i<params.dim();++i){
		params[i] = data[i];
	}
	delete[] data;
	DynMatrix<icl64f> x(1,xyz.rows());
	DynMatrix<icl64f> y(1,xyz.rows());
	DynMatrix<icl64f> z(1,xyz.rows());
	for(unsigned int i=0;i<xyz.rows();++i){
		x[i] = xyz(0,i);
		y[i] = xyz(1,i);
		z[i] = xyz(2,i);
	}
	DynMatrix<icl64f> *mat = SQFitter::sq.error(x,y,z);
	return mat;
}

DynMatrix<icl64f> *SQFitter::func_A(const DynMatrix<icl64f> &xyz, DynMatrix<icl64f> &params){
	SQFitter::sq.updateA(params);
	//this is for clipping
	double *data = SQFitter::sq.getA();
	for(unsigned int i=0;i<params.dim();++i){
		params[i] = data[i];
	}
	delete[] data;
	DynMatrix<icl64f> x(1,xyz.rows());
	DynMatrix<icl64f> y(1,xyz.rows());
	DynMatrix<icl64f> z(1,xyz.rows());
	for(unsigned int i=0;i<xyz.rows();++i){
		x[i] = xyz(0,i);
		y[i] = xyz(1,i);
		z[i] = xyz(2,i);
	}
	DynMatrix<icl64f> *mat = SQFitter::sq.error(x,y,z);
	return mat;
}

DynMatrix<icl64f> *SQFitter::func_EA(const DynMatrix<icl64f> &xyz, DynMatrix<icl64f> &params){
	SQFitter::sq.updateAE(params);
	//this is for clipping
	double *data = SQFitter::sq.getAE();
	for(unsigned int i=0;i<params.dim();++i){
		params[i] = data[i];
	}
	delete[] data;
	DynMatrix<icl64f> x(1,xyz.rows());
	DynMatrix<icl64f> y(1,xyz.rows());
	DynMatrix<icl64f> z(1,xyz.rows());
	for(unsigned int i=0;i<xyz.rows();++i){
		x[i] = xyz(0,i);
		y[i] = xyz(1,i);
		z[i] = xyz(2,i);
	}
	DynMatrix<icl64f> *mat = SQFitter::sq.error(x,y,z);
	return mat;
}
}
