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
#include <ICLUtils/Point32f.h>
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

	DynMatrix<icl64f> centeredImage;

	DynMatrix<icl64f> distMat;

	DynMatrix<icl64f> assignMat;

	double gamma;

	double beta;

	double betaFinal;

	static const double betaUpdate = 1.05;

	static const double betaZero = 0.0004;
#ifdef HAVE_QT
	ICLDrawWidget *dw;
#endif
	DynMatrix<icl64f> iAdj;
	DynMatrix<icl64f> wAdj;

	//expected or random pose
	//DynMatrix<icl64f> Q1;
	//expected or random pose
	//DynMatrix<icl64f> Q2;
	//squared distances
	DynMatrix<icl64f> d;

	DynMatrix<icl64f> w;
	//object points
	std::vector<DynMatrix<icl64f> > P;
	//image points
	std::vector<DynMatrix<icl64f> > p;

	DynMatrix<icl64f> L;
	DynMatrix<icl64f> invL;

	DynMatrix<icl64f> U;
	DynMatrix<icl64f> s;
	DynMatrix<icl64f> V;
	DynMatrix<icl64f> svdResult;

	DynMatrix<icl64f> eye2_2;

	DynMatrix<icl64f> r1T;
	DynMatrix<icl64f> r2T;
	DynMatrix<icl64f> r3T;

	DynMatrix<icl64f> projectedU;
	DynMatrix<icl64f> projectedV;
	DynMatrix<icl64f> replicatedProjectedU;
	DynMatrix<icl64f> replicatedProjectedV;

	DynMatrix<icl64f> col1;
	DynMatrix<icl64f> wkxj;
	DynMatrix<icl64f> col2;
	DynMatrix<icl64f> wkyj;

	DynMatrix<icl64f> pts2d;

	DynMatrix<icl64f> summedByColAssign;


	DynMatrix<icl64f>  weightedUi;
	DynMatrix<icl64f>  weightedVi;

	DynMatrix<icl64f> R1;
	DynMatrix<icl64f> R2;
	DynMatrix<icl64f> R3;

	DynMatrix<icl64f> ROT;

	DynMatrix<icl64f> T;
	double Tz;
	double Tx;
	double Ty;

	double f;

	double sumNonslack;

	double sum;

	double alpha;

	bool draw;

	DynMatrix<icl64f>& cross(DynMatrix<icl64f> &x, DynMatrix<icl64f> &y, DynMatrix<icl64f> &r);

	void maxPosRatio(DynMatrix<icl64f> &assignMat, DynMatrix<icl64f> &pos, DynMatrix<icl64f> &ratios);

	DynMatrix<icl64f> &sinkhornImp(DynMatrix<icl64f> &M);

	double cond(DynMatrix<icl64f> &A);

	double max(DynMatrix<icl64f> s);


public:
	SoftPosit();

	~SoftPosit();

	void init();

	DynMatrix<icl64f> getRotationMat(){
		return ROT;
	}

	DynMatrix<icl64f> getTranslation(){
		return T;
	}

	//unused
	int numMatches(DynMatrix<icl64f> &assignMat);

	void softPosit(DynMatrix<icl64f> imagePts, DynMatrix<icl64f> worldPts, double beta0, int noiseStd,	DynMatrix<icl64f> initRot,
				DynMatrix<icl64f> initTrans, double focalLength, DynMatrix<icl64f> center = DynMatrix<icl64f>(2,0), bool draw = true);
#ifdef HAVE_QT
	void softPosit(DynMatrix<icl64f> imagePts, DynMatrix<icl64f> imageAdj, DynMatrix<icl64f> worldPts,
			DynMatrix<icl64f> worldAdj, double beta0, int noiseStd,	DynMatrix<icl64f> initRot,
			DynMatrix<icl64f> initTrans, double focalLength, ICLDrawWidget &w,
			DynMatrix<icl64f> center = DynMatrix<icl64f>(2,0), bool draw = true);
#endif
	void softPosit(std::vector<Point32f> imagePts, std::vector<FixedColVector<double,3> > worldPts,
					double beta0, int noiseStd,	DynMatrix<icl64f> initRot, DynMatrix<icl64f> initTrans,
					double focalLength, DynMatrix<icl64f> center = DynMatrix<icl64f>(2,0));
#ifdef HAVE_QT
	void softPosit(std::vector<Point32f> imagePts, DynMatrix<icl64f> imageAdj, std::vector<FixedColVector<double,3> > worldPts,
				DynMatrix<icl64f> worldAdj, double beta0, int noiseStd,	DynMatrix<icl64f> initRot,
				DynMatrix<icl64f> initTrans, double focalLength, ICLDrawWidget &w,
				DynMatrix<icl64f> center = DynMatrix<icl64f>(2,0), bool draw=true);
#endif
	void proj3dto2d(DynMatrix<icl64f> pts3d, DynMatrix<icl64f> &rot, DynMatrix<icl64f> &trans,
				double flength, int objdim, DynMatrix<icl64f> &center, DynMatrix<icl64f> &pts2d);

	bool isNullMatrix(const DynMatrix<icl64f> &M){
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
	void visualize(const DynMatrix<icl64f> & imagePts, const DynMatrix<icl64f> &projWorldPts, unsigned int delay=200);

	void visualize(ICLDrawWidget &w,const DynMatrix<icl64f> & imagePts, const DynMatrix<icl64f> &imageAdj,
			const DynMatrix<icl64f> &projWorldPts, const DynMatrix<icl64f> &worldAdj, unsigned int delay=200);
#endif
};

}

#endif /* ICL_SOFTPOSIT_H_ */
