/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLGeom/src/SuperQuadric.cpp                           **
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
#include <ICLGeom/SuperQuadric.h>

namespace icl{

struct SuperQuadric::Data{
	double x;
	double y;
	double z;
	double e1;
	double e2;
	double a1;
	double a2;
	double a3;
	double rx;
	double ry;
	double rz;

	Data():x(0.0),y(0.0),z(0.0),e1(0.0),e2(0.0),a1(0.0),a2(0.0),a3(0.0),rx(0.0),ry(0.0),rz(0.0){}

	Data(double x_, double y_, double z_, double e_1, double e_2,
			double a_1, double a_2, double a_3, double r_x, double r_y, double r_z):
				x(x_),y(y_),z(z_),e1(e_1),e2(e_2),a1(a_1),a2(a_2),a3(a_3),rx(r_x),ry(r_y),rz(r_z){
	}

	~Data(){
	}
};

SuperQuadric::SuperQuadric(const DynMatrix<icl64f> &params,bool clip):m_data(new SuperQuadric::Data()){
	updateAll(params,clip);
}

SuperQuadric::SuperQuadric(double x, double y, double z, double e1, double e2,
		double a1, double a2, double a3, double rx, double ry, double rz):
		m_data(new SuperQuadric::Data(x,y,z,e1,e2,a1,a2,a3,rx,ry,rz)){
}

SuperQuadric::~SuperQuadric(){
	delete m_data;
}

double SuperQuadric::clip_val(double v, double vmin, double vmax){
	if(v<vmin)
		return vmin;
	else if(v>vmax)
		return vmax;
	else
		return v;
}

void SuperQuadric::updateAll(const DynMatrix<icl64f> &params, bool cl){
	m_data->x = params[0]; m_data->y = params[1]; m_data->z = params[2];
	m_data->e1 = params[3]; m_data->e2 = params[4];
	m_data->a1 = params[5]; m_data->a2 = params[6]; m_data->a3 = params[7];
	m_data->rx = params[8]; m_data->ry = params[9]; m_data->rz = params[10];
	if (cl)
		clip();
}

void SuperQuadric::updateAllR(const DynMatrix<icl64f> &params, bool cl){
	m_data->x = params[0]; m_data->y = params[1]; m_data->z = params[2];
	m_data->e1 = params[3]; m_data->e2 = params[4];
	m_data->a1 = params[5]; m_data->a2 = params[6]; m_data->a3 = params[7];
	if (cl)
		clip();
}

void SuperQuadric::updateAE(const DynMatrix<icl64f> &params, bool cl){
	m_data->e1 = params[0]; m_data->e2 = params[1];
	m_data->a1 = params[2]; m_data->a2 = params[3]; m_data->a3 = params[4];
	if (cl)
		clip();
}

void SuperQuadric::updateE(const DynMatrix<icl64f> &params, bool cl){
	m_data->e1 = params[0]; m_data->e2 = params[1];
	if (cl)
		clip();
}

void SuperQuadric::updateR(const DynMatrix<icl64f> &params, bool cl){
	m_data->rx = params[0]; m_data->ry = params[1]; m_data->rz = params[2];
	if (cl)
		clip();
}

void SuperQuadric::updateA(const DynMatrix<icl64f> &params, bool cl){
	m_data->a1 = params[0]; m_data->a2 = params[1]; m_data->a3 = params[2];
	if(cl)
		clip();
}

void SuperQuadric::updateXYZ(const DynMatrix<icl64f> &params, bool cl){
	m_data->x = params[0]; m_data->y = params[1]; m_data->z = params[2];
	if(cl)
		clip();
}

void SuperQuadric::clip(){
	m_data->e1 = clip_val(m_data->e1,E_MIN,E_MAX);
	m_data->e2 = clip_val(m_data->e2,E_MIN,E_MAX);
	m_data->a1 = clip_val(m_data->a1,A_MIN,A_MAX);
	m_data->a2 = clip_val(m_data->a2,A_MIN,A_MAX);
	m_data->a3 = clip_val(m_data->a3,A_MIN,A_MAX);

	//this seems not to be necessary with the implemented Levenberg-Marquardt
	//m_data->rx = fmod(m_data->rx, (2*M_PI));
	//m_data->ry = fmod(m_data->ry, (2*M_PI));
	//m_data->rz = fmod(m_data->rz, (2*M_PI));
}

void SuperQuadric::setXYZ(double x, double y, double z){
	m_data->x = x;
	m_data->y = y;
	m_data->z = z;
}

void SuperQuadric::seta1a2a3(double a1, double a2, double a3){
	m_data->a1 = clip_val(a1,A_MIN,A_MAX);
	m_data->a2 = clip_val(a2,A_MIN,A_MAX);
	m_data->a3 = clip_val(a3,A_MIN,A_MAX);
}

void SuperQuadric::sete1e2(double e1, double e2){
	m_data->e1 = clip_val(e1,E_MIN,E_MAX);
	m_data->e2 = clip_val(e2,E_MIN,E_MAX);
}

void SuperQuadric::setrxryrz(double rx, double ry, double rz){
	//clipping seems not to be necessary
	m_data->rx = rx;//fmod(rx, (2*M_PI));
	m_data->ry = ry;//fmod(ry, (2*M_PI));
	m_data->rz = rz;//fmod(rz, (2*M_PI));
}

DynMatrix<icl64f> SuperQuadric::screate_hom_transformation(double x, double y, double z,
		double rx, double ry, double rz){

	double a  = std::cos(rx);
	double b  = std::sin(rx);
	double c  = std::cos(ry);
	double d  = std::sin(ry);
	double e  = std::cos(rz);
	double f  = std::sin(rz);

	double ad = a*d;
	double bd = b*d;

	DynMatrix<icl64f> mat(4,4);
	mat[0] = c*e; mat[1] = -c*f; mat[2] = -d; mat[3] = x;
	mat[4] = -bd*e+a*f; mat[5] = bd*f+a*e; mat[6] = -b*c; mat[7] = y;
	mat[8] = ad*e+b*f; mat[9] = -ad*f+b*e; mat[10] = a*c; mat[11] = z;
	mat[12] = 0; mat[13] = 0; mat[14] = 0; mat[15] = 1;

	return mat;
}

DynMatrix<icl64f> SuperQuadric::create_hom_transformation(){
	return screate_hom_transformation(m_data->x,m_data->y,m_data->z,
			m_data->rx,m_data->ry,m_data->rz);
}

DynMatrix<icl64f> SuperQuadric::strans_pts(const double x, const double y, const double z, const DynMatrix<icl64f> &T){
	//transforms the passed points according to the passed transformation and returns them
	DynMatrix<icl64f> vec(1,4); vec[0] = x; vec[1] = y; vec[2] = z; vec[3] = 1;
	DynMatrix<icl64f> res = T*vec;
	return res;
}

DynMatrix<icl64f> SuperQuadric::distance(const DynMatrix<icl64f> &p_x, const DynMatrix<icl64f> &p_y,
		const DynMatrix<icl64f> &p_z, const double e1, const double e2,
		const double a1, const double a2, const double a3, const DynMatrix<icl64f> &T){
	/* 1 for points on the surface. If not 0.05 < e1,e2 < 2 or
        not 0.01 < a1,a2,a3 < 100 they are clipped to the borders of the
        intervals.*/

	DynMatrix<icl64f> dd(1,p_x.dim());

	for(unsigned int i=0;i<p_x.dim();++i){
		DynMatrix<icl64f> p = strans_pts(p_x[i],p_y[i],p_z[i],T);
		double px = p[0];
		double py = p[1];
		double pz = p[2];

		double d1 = std::pow((fabs(px)/a1),(2.0/e2));
		double d2 = pow((fabs(py)/a2),(2.0/e2));
		double d = pow( (d1 + d2),(e2/e1)) + pow((fabs(pz)/a3),(2.0/e1));
		//double d = pow( ( pow((fabs(px)/a1),(2.0/e2)) + pow((fabs(py)/a2),(2.0/e2)) ),(e2/e1)) +
		//	pow((fabs(pz)/a3),(2.0/e1));
		int sign = 0;
		if(d>0)
			sign = 1;
		else if(d<0)
			sign = -1;
		d=sign*pow(fabs(d),e1);
		dd[i] = d;
	}
	return dd;
}

DynMatrix<icl64f> SuperQuadric::distance(const DynMatrix<icl64f> &x, const DynMatrix<icl64f> &y, const DynMatrix<icl64f> &z){
	// 1 for points on the surface.
	DynMatrix<icl64f> mat = create_hom_transformation();
	DynMatrix<icl64f> mi = mat.pinv();
	DynMatrix<icl64f> dist = distance(x,y,z,m_data->e1,m_data->e2,m_data->a1,m_data->a2,m_data->a3, mi);
	return dist;
}

DynMatrix<icl64f> *SuperQuadric::error(const DynMatrix<icl64f> &x, const DynMatrix<icl64f> &y, const DynMatrix<icl64f> &z){
	DynMatrix<icl64f> dist = distance(x,y,z);
	DynMatrix<icl64f> *res = new DynMatrix<icl64f>(1,dist.rows());
	for(unsigned int i=0;i<dist.dim();++i){
		(*res)[i] = ((dist[i]) -1.0) * std::sqrt(m_data->a1*m_data->a2*m_data->a3);
	}
	return res;
}

double *SuperQuadric::getAllParams(){
	double *p = new double[11];
	p[0] = m_data->x; p[1] = m_data->y; p[2] = m_data->z;
	p[3] = m_data->e1; p[4] = m_data->e2;
	p[5] = m_data->a1; p[6] = m_data->a2; p[7] = m_data->a3;
	p[8] = m_data->rx; p[9] = m_data->ry; p[10] = m_data->rz;
	return p;
}

double *SuperQuadric::getR(){
	double *p = new double[3];
	p[0] = m_data->rx; p[1] = m_data->ry; p[2] = m_data->rz;
	return p;
}

double *SuperQuadric::getAllR(){
	double *p = new double[8];
	p[0] = m_data->x; p[1] = m_data->y; p[2] = m_data->z;
	p[3] = m_data->e1; p[4] = m_data->e2;
	p[5] = m_data->a1; p[6] = m_data->a2; p[7] = m_data->a3;
	return p;
}

double *SuperQuadric::getAE(){
	double *p = new double[5];
	p[0] = m_data->e1; p[1] = m_data->e2;
	p[2] = m_data->a1; p[3] = m_data->a2; p[4] = m_data->a3;
	return p;
}

double *SuperQuadric::getE(){
	double *p = new double[2];
	p[0] = m_data->e1; p[1] = m_data->e2;
	return p;
}

double *SuperQuadric::getA(){
	double *p = new double[3];
	p[0] = m_data->a1; p[1] = m_data->a2; p[2] = m_data->a3;
	return p;
}

double *SuperQuadric::getXYZ(){
	double *p = new double[3];
	p[0] = m_data->x; p[1] = m_data->y; p[2] = m_data->z;
	return p;
}

}
