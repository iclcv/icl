/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : include/ICLUtils/LMA.h                                 **
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
#ifndef ICL_LMA_H_
#define ICL_LMA_H_

#include <ICLUtils/BasicTypes.h>
#include <ICLUtils/DynMatrix.h>

#include <ICLUtils/Uncopyable.h>

namespace icl{

/**
 * Simple implementation of the Levenberg-Marquardt fitting algorithm.
 * This class also contains an implementation of a simple gradient descent.
 * In future this class should be extended with static variants, stepping functions
 * and the sparse version of the Levenberg-Marquardt algorithm.
 */
class LMA : public Uncopyable{

private:
	///computes the standard dotprodukt (scalarproduct) between to vecotrs
	/**
	 * @param a first vector for computation
	 * @param b second vector for computation
	 * @return dotprodukt of a and b
	 */
	double dot(DynMatrix<icl64f> &a, DynMatrix<icl64f> &b);
public:

	///empty contructor
	LMA();

	///destructor
	~LMA();

	/// simple gradient descent
	/**
	 * @param xyz inputdata for the function, where each column of the matrix represent one dimension,
	 * 						 so each line is a n-dimensional-data-tupel
	 * @param params contains values for parameters to be optimized
	 * @param func pointer to errorfunction where x is the inputdata and params the current parameters
	 * @param jacobian pointer to function for the jacobian matrix where x is the inputdata and params the current parameters
	 * @param error error to reach
	 * @param MaxIter maximum number of iterations
	 */
	void gradient_descent(const DynMatrix<icl64f> &xyz,DynMatrix<icl64f> &params,
			DynMatrix<icl64f> *(*func)(const DynMatrix<icl64f> &x, DynMatrix<icl64f> &params),
			DynMatrix<icl64f> *(*jacobian)(const DynMatrix<icl64f> &x, DynMatrix<icl64f> &params),
			double error=0.25, int MaxIter=100);


	/// simple Levenberg-Marquardt algorithm
	/**
	 * @param x inputdata for the function, where each column of the matrix represent one dimension,
	 * 			so each line is a n-dimensional-data-tupel
	 * @param func pointer to errorfunction where x is the inputdata and params the current parameters
	 * @param jacobian pointer to function for the jacobian matrix where x is the inputdata and params the current parameters
	 * @param guess contains values for parameters to be optimized
	 * @param thresh error to reach
	 * @param n_iters maximum number of iterations
	 * @return last evaluated error
	 */
	double solve(const DynMatrix<icl64f> &x,
			DynMatrix<icl64f> *(*func)(const DynMatrix<icl64f> &x, DynMatrix<icl64f> &params),
			DynMatrix<icl64f> *(*jacobian)(const DynMatrix<icl64f> &x, DynMatrix<icl64f> &params),
			DynMatrix<icl64f> &guess,const double thresh=0.25,const int n_iters = 1000, bool debug_output=false);

};
}

#endif /* ICL_LMA_H_ */
