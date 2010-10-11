/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : include/ICLAlgorithms/SoftPosit.h                      **
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

#ifndef ICL_SOFTPOSIT_H_
#define ICL_SOFTPOSIT_H_

#include <ICLUtils/DynMatrix.h>
#include <ICLUtils/FixedVector.h>
#include <ICLUtils/Macros.h>
#include <ICLUtils/Thread.h>
#ifdef HAVE_QT
#include<ICLQt/DrawWidget.h>
#endif

namespace icl{


class SoftPosit{
private:
	//M
	unsigned int nbWorldPts;
	//N
	unsigned int nbImagePts;

	DynMatrix<double> centeredImage;

	DynMatrix<double> distMat;

	DynMatrix<double> assignMat;

	double gamma;

	double beta;

	double betaFinal;

	static const double betaUpdate = 1.05;

	static const double betaZero = 0.0004;

	ICLDrawWidget *dw;

	DynMatrix<double> iAdj;
	DynMatrix<double> wAdj;

	//expected or random pose
	//DynMatrix<double> Q1;
	//expected or random pose
	//DynMatrix<double> Q2;
	//squared distances
	DynMatrix<double> d;

	DynMatrix<double> w;
	//object points
	std::vector<DynMatrix<double> > P;
	//image points
	std::vector<DynMatrix<double> > p;

	DynMatrix<double> L;
	DynMatrix<double> invL;

	DynMatrix<double> U;
	DynMatrix<double> s;
	DynMatrix<double> V;
	DynMatrix<double> svdResult;

	DynMatrix<double> eye2_2;

	DynMatrix<double> r1T;
	DynMatrix<double> r2T;
	DynMatrix<double> r3T;

	DynMatrix<double> projectedU;
	DynMatrix<double> projectedV;
	DynMatrix<double> replicatedProjectedU;
	DynMatrix<double> replicatedProjectedV;

	DynMatrix<double> col1;
	DynMatrix<double> wkxj;
	DynMatrix<double> col2;
	DynMatrix<double> wkyj;

	DynMatrix<double> pts2d;

	DynMatrix<double> summedByColAssign;


	DynMatrix<double>  weightedUi;
	DynMatrix<double>  weightedVi;

	DynMatrix<double> R1;
	DynMatrix<double> R2;
	DynMatrix<double> R3;

	DynMatrix<double> ROT;

	DynMatrix<double> T;
	double Tz;
	double Tx;
	double Ty;

	double f;

	double sumNonslack;

	double sum;

	double alpha;

	bool draw;

	DynMatrix<double>& cross(DynMatrix<double> &x, DynMatrix<double> &y, DynMatrix<double> &r);

	void maxPosRatio(DynMatrix<double> &assignMat, DynMatrix<double> &pos, DynMatrix<double> &ratios);

	DynMatrix<double> &sinkhornImp(DynMatrix<double> &M);

	double cond(DynMatrix<double> &A);

	double max(DynMatrix<double> s);


public:
	SoftPosit();

	~SoftPosit();

	void init();

	DynMatrix<double> getRotationMat(){
		return ROT;
	}

	DynMatrix<double> getTranslation(){
		return T;
	}

	//unused
	int numMatches(DynMatrix<double> &assignMat);

	void softPosit(DynMatrix<double> imagePts, DynMatrix<double> worldPts, double beta0, int noiseStd,	DynMatrix<double> initRot,
				DynMatrix<double> initTrans, double focalLength, DynMatrix<double> center = DynMatrix<double>(2,0), bool draw = true);

	void softPosit(DynMatrix<double> imagePts, DynMatrix<double> imageAdj, DynMatrix<double> worldPts,
			DynMatrix<double> worldAdj, double beta0, int noiseStd,	DynMatrix<double> initRot,
			DynMatrix<double> initTrans, double focalLength, ICLDrawWidget &w,
			DynMatrix<double> center = DynMatrix<double>(2,0), bool draw = true);

	void softPosit(std::vector<Point32f> imagePts, std::vector<FixedColVector<double,3> > worldPts,
					double beta0, int noiseStd,	DynMatrix<double> initRot, DynMatrix<double> initTrans,
					double focalLength, DynMatrix<double> center = DynMatrix<double>(2,0));

	void softPosit(std::vector<Point32f> imagePts, DynMatrix<double> imageAdj, std::vector<FixedColVector<double,3> > worldPts,
				DynMatrix<double> worldAdj, double beta0, int noiseStd,	DynMatrix<double> initRot,
				DynMatrix<double> initTrans, double focalLength, ICLDrawWidget &w,
				DynMatrix<double> center = DynMatrix<double>(2,0), bool draw=true);

	void proj3dto2d(DynMatrix<double> pts3d, DynMatrix<double> &rot, DynMatrix<double> &trans,
				double flength, int objdim, DynMatrix<double> &center, DynMatrix<double> &pts2d);

	bool isNullMatrix(const DynMatrix<double> &M){
		bool isNull = true;
		for(unsigned int i=0;i<M.cols();++i){
			for(unsigned int j=0;j<M.rows();++j){
				if(M[i+j*M.cols()] != 0){
					isNull = false;
					return isNull;
				}
			}
		}
		return isNull;
	}

#ifdef HAVE_QT
	void visualize(const DynMatrix<double> & imagePts, const DynMatrix<double> &projWorldPts, unsigned int delay=200);

	void visualize(ICLDrawWidget &w,const DynMatrix<double> & imagePts, const DynMatrix<double> &imageAdj,
			const DynMatrix<double> &projWorldPts, const DynMatrix<double> &worldAdj, unsigned int delay=200);
#endif
};

}

#endif /* ICL_SOFTPOSIT_H_ */
