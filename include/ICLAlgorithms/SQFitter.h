/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : include/ICLAlgorithms/SQFitter.h                       **
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
#ifndef ICL_SQFITTER_H_
#define ICL_SQFITTER_H_

#include <ICLUtils/BasicTypes.h>
#include <ICLUtils/DynMatrix.h>

#include <ICLGeom/SuperQuadric.h>
#include <ICLAlgorithms/LMA.h>
namespace icl{

/**
 * Implementation of automatic fitting of a superquadric to given pointcloud.
 * Internally Levenberg-Marquardt algorithm is used.
 */
class SQFitter{

private:
	///internal datastorage
	class Data;
	Data *m_data;

	///Superquadric
	static SuperQuadric sq;

	///min value of matrix
	/**
	 * @param m matrix
	 * @return min value of matrix
	 */
	double min(const DynMatrix<icl64f> &m);

	///max value of matrix
	/**
	 * @param m matrix
	 * @return max value of matrix
	 */
	double max(const DynMatrix<icl64f> &m);

	///mean of matrix
	/**
	 * @param m matrix
	 * @return meanvalue
	 */
	double mean(const DynMatrix<icl64f> &m);

	///sorts rotationmatrix with given eigenvalues
	/**
	 * @param w eigenvalues of R
	 * @param R rotationmatrix
	 * @return sorted rotationmatrix
	 */
	DynMatrix<icl64f> sort_eig(DynMatrix<icl64f> &w, DynMatrix<icl64f> &R);

	///computes initial values for optimization
	/**
	 * @params x points for xdim
	 * @params y points for ydim
	 * @params z points for zdim
	 */
	DynMatrix<icl64f> initial_estimate(const DynMatrix<icl64f> &x, const DynMatrix<icl64f> &y, const DynMatrix<icl64f> &z);

	///computes Euler angles from a given rotation matrix
	/**
	 * @param R rotationmatrix
	 * @return colvector of Euler angles
	 */
	DynMatrix<icl64f> euler_from_rot_matrix(DynMatrix<icl64f> &R);

public:

	///Constructor
	/**
	 * @param data points to fit, rowwise
	 */
	SQFitter(const DynMatrix<icl64f> *data);

	///Destructor
	~SQFitter();

	///fits superquadric to given data
	/**
	 * @param params initial params, can be all zero, order is x,y,z,e1,e2,a1,a2,a3,rx,ry,rz
	 * @param thresh max error
	 * @param maxiter maximum nuber of iteration to do
	 * @param autoinit automacally initializes params if true
	 */
	void fit(DynMatrix<icl64f> &params, const double thresh=0.01, const int maxiter=100, bool autoinit = true);

	///evaluates errorfunction of superquadric for x,y,z,e1,e2,a1,a2,a3,rx,ry,rz
	/**
	 * @param xyz points to fit
	 * @param params params to evaluate the errorfunction of superquadric on, order is x,y,z,e1,e2,a1,a2,a3,rx,ry,rz
	 * @return the error for each point to fit
	 */
	static DynMatrix<icl64f> *func_real(const DynMatrix<icl64f> &xyz, DynMatrix<icl64f> &params);

	///forward differences approximating the jacobian matrix of the errorfunction of superquadric
	/**
	 * Can be used in combination with every function given in this class.
	 * @param xyz points to fit, rowwise
	 * @param params new values for params, order is x,y,z,e1,e2,a1,a2,a3,rx,ry,rz
	 * @return jacobian Matrix evaluated at given points rowwise
	 */
	static DynMatrix<icl64f>* jacobian(const DynMatrix<icl64f> &xyz, DynMatrix<icl64f> &params);

	///evaluates errorfunction of superquadric for new params x,y,z
	/**
	 * @param xyz points to fit, rowwise
	 * @param params new params x,y,z
	 * @return Matrix of error
	 */
	static DynMatrix<icl64f> *func_XYZ(const DynMatrix<icl64f> &xyz, DynMatrix<icl64f> &params);

	///evaluates errorfunction of superquadric for new params rx,ry,rz
	/**
	 * @param xyz points to fit, rowwise
	 * @param params new params rx,ry,rz
	 * @return Matrix of error
	 */
	static DynMatrix<icl64f> *func_R(const DynMatrix<icl64f> &xyz, DynMatrix<icl64f> &params);

	///evaluates errorfunction of superquadric for new params e1,e2
	/**
	 * @param xyz points to fit, rowwise
	 * @param params new params e1,e2
	 * @return Matrix of error
	 */
	static DynMatrix<icl64f> *func_E(const DynMatrix<icl64f> &xyz, DynMatrix<icl64f> &params);

	///evaluates errorfunction of superquadric for new params a1,a2,a3
	/**
	 * @param xyz points to fit, rowwise
	 * @param params new params a1,a2,a3
	 * @return Matrix of error
	 */
	static DynMatrix<icl64f> *func_A(const DynMatrix<icl64f> &xyz, DynMatrix<icl64f> &params);

	///evaluates errorfunction of superquadric for new params e1,e2,a1,a2,a3
	/**
	 * @param xyz points to fit, rowwise
	 * @param params new params e1,e2,a1,a2,a3
	 * @return Matrix of error
	 */
	static DynMatrix<icl64f> *func_EA(const DynMatrix<icl64f> &xyz, DynMatrix<icl64f> &params);
};
}

#endif /* ICL_SQFITTER_H_ */
