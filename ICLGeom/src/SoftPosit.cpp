/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLAlgorithms/src/SoftPosit.cpp                        **
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
#include<ICLGeom/SoftPosit.h>

using namespace icl;
namespace icl{

#ifdef HAVE_QT
SoftPosit::SoftPosit():dw(0){
	ROT.setBounds(3,3);
	T.setBounds(1,3);
	R1.setBounds(1,3);
	R2.setBounds(1,3);
	R3.setBounds(1,3);
	eye2_2.setBounds(2,2);
	eye2_2.at(0,0) = 1.0;
	eye2_2.at(1,1) = 1.0;
	draw = false;
}
#else
SoftPosit::SoftPosit(){
	ROT.setBounds(3,3);
	T.setBounds(1,3);
	R1.setBounds(1,3);
	R2.setBounds(1,3);
	R3.setBounds(1,3);
	eye2_2.setBounds(2,2);
	eye2_2.at(0,0) = 1.0;
	eye2_2.at(1,1) = 1.0;
	draw = false;
}
#endif
SoftPosit::~SoftPosit(){}

void SoftPosit::init(){
	nbWorldPts = 7;
	nbImagePts = 8;
	Tz = 0.0;
	Tx = 0.0;
	Ty = 0.0;
	beta = betaZero;
	betaFinal=0.5;

	//////////////////////////////
	alpha = 1.0; //9.21*noiseStd*noiseStd + 1;
}

void SoftPosit::softPosit(DynMatrix<double> imagePts, DynMatrix<double> worldPts, double beta0, int noiseStd,	DynMatrix<double> initRot,
		DynMatrix<double> initTrans, double focalLength, DynMatrix<double> center, bool draw){
	col1.setBounds(1,imagePts.rows());
	col2.setBounds(1,imagePts.rows());
	betaFinal=0.5;

	// Max allowed error per world point
	int maxDelta = sqrt(alpha)/2;
	// Update rate on beta.
	double epsilon0 = 0.01;                  // % Used to initialize assignement matrix.

	int minBetaCount = 20;

	nbImagePts = imagePts.rows();
	nbWorldPts = worldPts.rows();
	distMat=DynMatrix<double>(nbWorldPts,nbImagePts);
	//das ist dann gamma
	double max = iclMax(nbImagePts,nbWorldPts);
	double scale = 1.0/(max + 1);

	centeredImage.setBounds(imagePts.cols(),imagePts.rows());
	for(unsigned int i=0;i<imagePts.rows();++i){
		centeredImage.at(0,i) =(imagePts.at(0,i)-center.at(0,0))/focalLength;
		centeredImage.at(1,i) =(imagePts.at(1,i)-center.at(1,0))/focalLength;
	}
	DynMatrix<double> imageOnes(1,nbImagePts,1);
	DynMatrix<double> homogeneousWorldPts(4,nbWorldPts) ;
	for(unsigned int i=0;i<nbWorldPts;++i){
		for(unsigned int j=0;j<3;++j){
			homogeneousWorldPts.at(j,i) = worldPts.at(j,i);
		}
		homogeneousWorldPts.at(3,i) = 1.0;
	}
	//Initial rotation and translation as passed into this function.
	ROT = initRot;
	T = initTrans;

	//Initialize the depths of all world points based on initial pose.

	DynMatrix<double> temp(1,4,1);
	for(unsigned int i=0;i<3;++i){
		temp.at(0,i)=ROT.at(i,2)/T.at(0,2);
	}
	DynMatrix<double> wk = homogeneousWorldPts * temp;

#ifdef HAVE_QT
	//DynMatrix<double> projWorldPts = proj3dto2d(worldPts, rot, trans, focalLength, 1, center);
	if(draw){
		proj3dto2d(worldPts, ROT, T, focalLength,1,center,pts2d);
		//visualize(w,imagePts, imageAdj,	pts2d, worldAdj);
		visualize(imagePts, pts2d);
	}
#endif
	//First two rows of the camera matrices (for both perspective and SOP).  Note:
	//the scale factor is s = f/Tz = 1/Tz since f = 1.  These are column 4-vectors.
	double t1[] = {ROT(0,0)/T.at(0,2),ROT(1,0)/T.at(0,2),ROT(2,0)/T.at(0,2), T.at(0,0)/T.at(0,2)};
	double t2[] = {ROT(0,1)/T.at(0,2),ROT(1,1)/T.at(0,2),ROT(2,1)/T.at(0,2), T.at(0,1)/T.at(0,2)};
	r1T = DynMatrix<double>(1,4,t1);
	r2T = DynMatrix<double>(1,4,t2);
	r3T.setBounds(1,4);
	int betaCount = 0;
	int poseConverged = 0;
	int assignConverged = 0;
	int foundPose = 0;
	beta = beta0;
	DynMatrix<double> r1Tr2T(2,3);
	assignMat = DynMatrix<double>(nbWorldPts+1,nbImagePts+1,1+epsilon0);

	while ((beta < betaFinal) && !assignConverged){
		projectedU = homogeneousWorldPts * r1T;
		projectedV = homogeneousWorldPts * r2T;
		replicatedProjectedU = imageOnes * projectedU.transp();
		replicatedProjectedV = imageOnes * projectedV.transp();
		col1 = centeredImage.col(0);
		wkxj = col1 * wk.transp();
		col2 = centeredImage.col(1);
		wkyj = col2 * wk.transp();

		//TODO optimize
		DynMatrix<double> temp1 = replicatedProjectedU - wkxj;
		DynMatrix<double> temp2 = replicatedProjectedV - wkyj;
		for(unsigned int i=0;i<nbWorldPts;++i){
			for(unsigned j=0;j<nbImagePts;++j){
				distMat.at(i,j) = focalLength*focalLength*(temp1(i,j)*temp1(i,j)+temp2(i,j)*temp2(i,j));
			}
		}
		for(unsigned int i=0;i<nbWorldPts;++i){
			for(unsigned int j=0;j<nbImagePts;++j){
				assignMat.at(i,j) = scale * ( std::exp( -beta*( distMat(i,j) - alpha ) ) );
			}
		}
		//assignMat(1:nbImagePts+1,nbWorldPts+1) = scale;
		for(unsigned int i=0;i<nbImagePts+1;++i){
			assignMat.at(nbWorldPts,i) = scale;
		}
		//assignMat(nbImagePts+1,1:nbWorldPts+1) = scale;
		for(unsigned int i=0;i<nbWorldPts+1;++i){
			assignMat.at(i,nbImagePts) = scale;
		}
		sinkhornImp(assignMat);

		//int numMatchPts = numMatches(assignMat);

		//sumNonslack = sum(sum(assignMat(1:nbImagePts,1:nbWorldPts)));
		sumNonslack=0.0;
		for(unsigned int i=0;i<nbWorldPts;++i){
			for(unsigned int j=0;j<nbImagePts;++j){
				sumNonslack += assignMat(i,j);
			}
		}
		//summedByColAssign = sum(assignMat(1:nbImagePts, 1:nbWorldPts), 1);
		summedByColAssign = DynMatrix<double>(nbWorldPts,1);
		for(unsigned int i=0;i<nbWorldPts;++i){
			for(unsigned int j=0;j<nbImagePts;++j){
				summedByColAssign.at(i,0) += assignMat(i,j);
			}
		}

		//TODO optimize
		L = DynMatrix<double>(4,4);
		DynMatrix<double> temp11(1,4);
		DynMatrix<double> temp22(4,1);
		DynMatrix<double> temp33(4,4);
		DynMatrix<double> temp44(4,4);
		for (unsigned int k = 0;k<nbWorldPts;k++){
			for(int i=0;i<4;++i){
				temp11.at(0,i) = homogeneousWorldPts.at(i,k);
				temp22.at(i,0) = homogeneousWorldPts.at(i,k);
			}
			//sumSkSkT = sumSkSkT + summedByColAssign.at(k,1) * homogeneousWorldPts(k,:)' * homogeneousWorldPts(k,:);
			temp33 = temp11 * temp22;
			temp33.mult(summedByColAssign.at(k,0),temp44);
			L = L + temp44;
		}
		if (cond(L) > 1e10){
			std::cout << "cond L to small\n";
			return;
		}
		invL = L.inv();

		//poseConverged = 0;
		//TODO optimize
		weightedUi = DynMatrix<double>(1,4);
		weightedVi = DynMatrix<double>(1,4);

		DynMatrix<double>  temp55(1,4);
		DynMatrix<double>  temp66(1,4);
		for(unsigned int j = 0;j<nbImagePts;++j){
			for(unsigned int k = 0;k<nbWorldPts;++k){
				for(int i=0;i<4;++i){
					temp55.at(0,i) = homogeneousWorldPts.at(i,k);
				}
				temp55.mult(assignMat(k,j) * wk(k,0) * centeredImage(0,j), temp66);
				weightedUi = weightedUi + temp66;
				temp55.mult(assignMat(k,j) * wk(k,0) * centeredImage(1,j), temp66);
				weightedVi = weightedVi + temp66;
			}
		}
		// % Compute the pose vectors. M = s(R1,Tx) and N = s(R2,Ty) where the
		//% scale factor is s = f/Tz, and f = 1.  These are column 4-vectors.
		r1T = invL * weightedUi;
		r2T = invL * weightedVi;

		for(unsigned int i=0;i<3;++i){
			r1Tr2T.at(0,i) = r1T.at(0,i);
			r1Tr2T.at(1,i) = r2T.at(0,i);
		}

		if (1) {//calculation of R and T.
			r1Tr2T.svd(U,s,V);
			svdResult = U * eye2_2 * V.transp();
			for(unsigned int i=0;i<3;++i){
				R1.at(0,i) = svdResult.at(0,i);
				R2.at(0,i) = svdResult.at(1,i);
			}
			cross(R1,R2,R3);
			Tz = 2 / (s.at(0,0) + s.at(0,1));
			Tx = r1T.at(0,3) * Tz;
			Ty = r2T.at(0,3) * Tz;
			r3T.at(0,0)=R3.at(0,0);
			r3T.at(0,1)=R3.at(0,1);
			r3T.at(0,2)=R3.at(0,2);
			r3T.at(0,3)=Tz;
		}else{
			//TODO implement me
			/* % Standard calculation of R and T.  The rotation matrix may not be
            % orthonormal.  The object must distort when the rotation matrix
            % is not orthonormal.
            r1TSquared = r1T(1)*r1T(1) + r1T(2)*r1T(2)+ r1T(3)*r1T(3);
            r2TSquared = r2T(1)*r2T(1) + r2T(2)*r2T(2)+ r2T(3)*r2T(3);
            Tz = sqrt(2/(r1TSquared+r2TSquared));  // % Chang & Tsai's suggestion.
            r1N = r1T*Tz;                   % Column 4-vectors: (R1,Tx).
            r2N = r2T*Tz;                   %                   (R2,Ty).
            r1 = r1N(1:3);                  % Three rows of the rotation matrix.
            r2 = r2N(1:3);
            r3 = cross(r1,r2);
            r3T= [r3; Tz];                  % Column 4-vector: (R3,Tz).
            Tx = r1N(4);
            Ty = r2N(4);*/
		}
		r1T.at(0,0) = R1.at(0,0)/Tz;
		r1T.at(0,1) = R1.at(0,1)/Tz;
		r1T.at(0,2) = R1.at(0,2)/Tz;
		r1T.at(0,3) = Tx/Tz;
		r2T.at(0,0) = R2.at(0,0)/Tz;
		r2T.at(0,1) = R2.at(0,1)/Tz;
		r2T.at(0,2) = R2.at(0,2)/Tz;
		r2T.at(0,3) = Ty/Tz;
		//TODO
		DynMatrix<double> temp001(1,3);
		r3T.mult(1/Tz,temp001);
		wk = homogeneousWorldPts * temp001;
		//delta = sqrt(sum(sum(assignMat(1:nbImagePts,1:nbWorldPts) .* distMat))/nbWorldPts);
		temp001.setBounds(nbWorldPts,nbImagePts,false);
		for(unsigned int i=0;i<nbWorldPts;++i){
			for(unsigned int j=0;j<nbImagePts;++j){
				temp001.at(i,j) = assignMat.at(i,j)*distMat.at(i,j);
			}
		}
		sum = 0.0;
		for(unsigned int i=0;i<temp001.cols();++i){
			for(unsigned int j=0;j<temp001.rows();++j){
				sum += temp001.at(i,j);
			}
		}
		double delta = sqrt(sum/nbWorldPts);
		if(delta < maxDelta)
			poseConverged = 1;
		else
			poseConverged = 0;

		beta = betaUpdate * beta;
		betaCount = betaCount + 1;
		if(poseConverged && betaCount>minBetaCount)
			assignConverged = 1;
		else
			assignConverged = 0;

		T.at(0,0) = Tx;
		T.at(0,1) = Ty;
		T.at(0,2) = Tz;

		for(unsigned int i=0;i<3;++i){
			ROT.at(i,0) = R1.at(0,i);
			ROT.at(i,1) = R2.at(0,i);
			ROT.at(i,2) = R3.at(0,i);
		}
		if(delta < maxDelta && betaCount > minBetaCount)
			foundPose = 1;
		else
			foundPose = 0;
#ifdef HAVE_QT
		if(draw){
			proj3dto2d(worldPts, ROT, T, focalLength, 1, center,pts2d);
			//visualize(w,imagePts, imageAdj,	pts2d, worldAdj);
			visualize(imagePts,pts2d);
		}
#endif
	}
	//SHOW(ROT);
	//SHOW(T);
}
#ifdef HAVE_QT
void SoftPosit::softPosit(DynMatrix<double> imagePts, DynMatrix<double> imageAdj, DynMatrix<double> worldPts,
		DynMatrix<double> worldAdj, double beta0, int noiseStd,	DynMatrix<double> initRot,
		DynMatrix<double> initTrans, double focalLength, ICLDrawWidget &w,
		DynMatrix<double> center, bool draw){
	dw = &w;
	iAdj = imageAdj;
	wAdj = worldAdj;
	softPosit(imagePts, worldPts, beta0, noiseStd, initRot, initTrans, focalLength, center);
}
#endif
void SoftPosit::softPosit(std::vector<Point32f> imagePts, std::vector<FixedColVector<double,3> > worldPts,
		double beta0, int noiseStd,	DynMatrix<double> initRot, DynMatrix<double> initTrans,
		double focalLength, DynMatrix<double> center){
	DynMatrix<double> imagePt(2,imagePts.size());
	for(unsigned int i=0; i<imagePts.size();++i){
		imagePt.at(0,i) = imagePts.at(i).x;
		imagePt.at(1,i) = imagePts.at(i).y;
	}
	DynMatrix<double> worldPt(3,worldPts.size());
	for(unsigned int i=0; i<worldPts.size();++i){
		worldPt.at(0,i) = worldPts.at(i).at(0,0);
		worldPt.at(1,i) = worldPts.at(i).at(0,1);
		worldPt.at(2,i) = worldPts.at(i).at(0,2);
	}

	softPosit(imagePt, worldPt, beta0, noiseStd, initRot, initTrans, focalLength, center, draw);
}
#ifdef HAVE_QT
void SoftPosit::softPosit(std::vector<Point32f> imagePts, DynMatrix<double> imageAdj, std::vector<FixedColVector<double,3> > worldPts,
		DynMatrix<double> worldAdj, double beta0, int noiseStd,	DynMatrix<double> initRot,
		DynMatrix<double> initTrans, double focalLength, ICLDrawWidget &w, DynMatrix<double> center,bool draw){

	DynMatrix<double> imagePt(2,imagePts.size());
	for(unsigned int i=0; i<imagePts.size();++i){
		imagePt.at(0,i) = imagePts.at(i).x;
		imagePt.at(1,i) = imagePts.at(i).y;
	}
	DynMatrix<double> worldPt(3,worldPts.size());
	for(unsigned int i=0; i<worldPts.size();++i){
		worldPt.at(0,i) = worldPts.at(i).at(0,0);
		worldPt.at(1,i) = worldPts.at(i).at(0,1);
		worldPt.at(2,i) = worldPts.at(i).at(0,2);
	}

	softPosit(imagePt, imageAdj, worldPt, worldAdj, beta0, noiseStd, initRot, initTrans, focalLength, w, center, draw);
}
#endif

DynMatrix<double>& SoftPosit::cross(DynMatrix<double> &x, DynMatrix<double> &y, DynMatrix<double> &r){
	if(x.cols()==1 && y.cols()==1 && x.rows()==3 && y.rows()==3){
		r.at(0,0) = x.at(0,1)*y.at(0,2)-x.at(0,2)*y.at(0,1);
		r.at(0,1) = x.at(0,2)*y.at(0,0)-x.at(0,0)*y.at(0,2);
		r.at(0,2) = x.at(0,0)*y.at(0,1)-x.at(0,1)*y.at(0,0);
	}
	return r;
}

void SoftPosit::proj3dto2d(DynMatrix<double> pts3d, DynMatrix<double> &rot, DynMatrix<double> &trans,
		double flength, int objdim, DynMatrix<double> &center, DynMatrix<double> &pts2d){
	//3D point matrix must be 3xN.
	if (objdim == 1){
		pts3d = pts3d.transp();
	}
	//number of 3D points.
	unsigned int numpts = pts3d.cols();
	DynMatrix<double> newtrans(numpts,3);
	for(unsigned int i=0;i<numpts;++i){
		newtrans.at(i,0) = trans.at(0,0);
		newtrans.at(i,1) = trans.at(0,1);
		newtrans.at(i,2) = trans.at(0,2);
	}
	DynMatrix<double> campts = rot*pts3d+newtrans;
	for(unsigned int i=0;i<campts.cols();++i){
		if(campts.at(i,2)<1e-20){
			campts.at(i,2) = 1e-20;
		}
	}
	pts2d.setBounds(campts.cols(),2);
	for(unsigned int i=0;i<campts.cols();++i){
		pts2d.at(i,0) = flength * campts.at(i,0)*(1/campts.at(i,2));
		pts2d.at(i,1) = flength * campts.at(i,1)*(1/campts.at(i,2));
	}
	DynMatrix<double> cent(numpts,2);
	for(unsigned int i=0;i<numpts;++i){
		cent.at(i,0) = center.at(0,0);
		cent.at(i,1) = center.at(1,0);
	}
	pts2d = pts2d +cent;
	if (objdim == 1){
		pts2d = pts2d.transp();
	}
}

void SoftPosit::maxPosRatio(DynMatrix<double> &assignMat, DynMatrix<double> &pos, DynMatrix<double> &ratios){
	unsigned int nrows = assignMat.rows()-1;
	double vmax =0.0;
	unsigned int imax=0;
	bool isMaxInRow = true;
	double cr = 0.0;
	double rr = 0.0;
	for(unsigned int k = 0;k<assignMat.cols()-1;++k){
		vmax = assignMat.at(k,0);
		imax = 0;
		for(unsigned int i=1;i<assignMat.rows();++i){
			if(vmax < assignMat.at(k,i)){
				vmax = assignMat.at(k,i);
				imax = i;
			}
		}
		if (imax == nrows)
			continue;
		isMaxInRow = true;
		for(unsigned int i=0;i<assignMat.cols();++i){
			if(vmax < assignMat.at(i,imax) && i != k)
				isMaxInRow = false;
		}
		if(isMaxInRow){
			pos.setBounds(2,pos.rows()+1);
			pos.at(0,pos.rows()-1) = imax;
			pos.at(1,pos.rows()-1) = k;
			rr = assignMat.at(assignMat.cols()-1,imax)/assignMat.at(k,imax);
			cr = assignMat.at(k,assignMat.rows()-1)/assignMat.at(k,imax);
			ratios.setBounds(2,ratios.rows()+1);
			ratios.at(0,ratios.rows()-1) = rr;
			ratios.at(1,ratios.rows()-1) = cr;
		}
	}
}

//TODO maybe implement normal/standard sinkhorn
DynMatrix<double> &SoftPosit::sinkhornImp(DynMatrix<double> &M){
	int iMaxIterSinkhorn=60;
	double fEpsilon2 = 0.001;
	int iNumSinkIter = 0;
	unsigned int nbRows = M.rows();
	unsigned int nbCols = M.cols();
	double fMdiffSum = fEpsilon2 + 1;
	DynMatrix<double> ratios;
	DynMatrix<double> posmax;
	maxPosRatio(M,posmax,ratios);
	DynMatrix<double> Mprev;
	DynMatrix<double> McolSums;
	DynMatrix<double> MrowSums;
	DynMatrix<double> ones(1,M.cols(),1.0);
	DynMatrix<double> MrowSumsRep;
	DynMatrix<double> McolSumsRep;
	while(std::abs(fMdiffSum) > fEpsilon2 && iNumSinkIter < iMaxIterSinkhorn){
		McolSums = DynMatrix<double>(M.cols(),1);
		MrowSums = DynMatrix<double>(1,M.rows());
		Mprev = M;
		for(unsigned int j=0;j<M.cols();++j)
			for(unsigned int i=0;i<M.rows();++i){
				McolSums.at(j,0) += M.at(j,i);
			}

		McolSums.at(nbCols-1,0) = 1;
		ones.setBounds(1,M.cols(),false,1.0);
		McolSumsRep = ones * McolSums;
		for(unsigned int i=0; i<M.cols();++i){
			for(unsigned j=0; j<M.rows();++j){
				M.at(i,j) = M.at(i,j)/McolSumsRep.at(i,j);
			}
		}
		for(unsigned int i=0;i<posmax.rows();++i){
			M.at(nbCols-1,posmax(0,i)) = ratios(0,i)*M(posmax(0,i),posmax(1,i));
		}
		for(unsigned int j=0;j<M.rows();++j)
			for(unsigned int i=0;i<M.cols();++i){
				MrowSums.at(0,j) += M.at(i,j);
			}
		MrowSums.at(0,nbRows-1) = 1;
		ones.setBounds(nbCols,1,false,1.0);
		MrowSumsRep = MrowSums*ones;
		for(unsigned int i=0; i<M.cols();++i){
			for(unsigned j=0; j<M.rows();++j){
				M.at(i,j) = M.at(i,j)/MrowSumsRep.at(i,j);
			}
		}
		for(unsigned int i=0;i<posmax.rows();++i){
			M(nbRows,posmax(1,i)) = ratios(2,i)*M(posmax(1,i),posmax(2,i));
		}
		iNumSinkIter=iNumSinkIter+1;
		fMdiffSum = 0.0;
		for(unsigned int i=0;i<M.cols();++i){
			for(unsigned int j=0;j<M.rows();++j){
				fMdiffSum += std::abs(M.at(i,j)-Mprev.at(i,j));
			}
		}
	}
	return M;
}

int SoftPosit::numMatches(DynMatrix<double> &assignMat){
		int num = 0;
		int nrows = assignMat.rows();
		double vmax = 0.0;
		int imax = 0;
		bool isMaxInRow = true;
		for(unsigned int k = 0 ;k<assignMat.cols();++k){
			vmax = assignMat.at(k,0);
			imax = 0;
			for(unsigned int i=1;i<assignMat.rows();++i){
				if(vmax < assignMat.at(k,i)){
					vmax = assignMat.at(k,i);
					imax = i;
				}
			}
			if (imax == nrows){
				continue;
			}
			isMaxInRow = true;
			for(unsigned int i=0;i<assignMat.cols();++i){
				if(vmax < assignMat.at(i,imax) && i != k)
					isMaxInRow = false;
			}
			if(isMaxInRow){
				num = num + 1;
			}
		}
		return num;
	}

double SoftPosit::cond(DynMatrix<double> &A){
	DynMatrix<double> U;
	DynMatrix<double> V;
	DynMatrix<double> s;
	A.svd(U,s,V);
	double n1 = max(s);
	(A.inv()).svd(U,s,V);
	double n2 = max(s);
	return n1 * n2;
}

double SoftPosit::max(DynMatrix<double> s){
		double max = s.at(0,0);
		for(unsigned int i=0;i<s.rows();++i){
			if(max < s.at(0,i))
				max = s.at(0,i);
		}
		return max;
	}

#ifdef HAVE_QT

void SoftPosit::visualize(const DynMatrix<double> & imagePts, const DynMatrix<double> &projWorldPts, unsigned int delay){
	dw->color(255,0,0,1);
	dw->linewidth(2);
	dw->lock();
	dw->reset();
	float offsetx=dw->size().rwidth()/2.0;
	float offsety=dw->size().rheight()/2.0;
	for(unsigned int i=0;i<wAdj.cols();++i){
		for(unsigned int j=0;j<wAdj.rows();++j){
			if(wAdj.at(i,j)==1){
				dw->line(projWorldPts.at(0,i)+offsetx, projWorldPts.at(1,i)+offsety,
						projWorldPts.at(0,j)+offsetx, projWorldPts.at(1,j)+offsety);
			}
		}
	}
	dw->color(0,0,255,255);
	for(unsigned int i=0;i<iAdj.cols();++i){
		for(unsigned int j=0;j<iAdj.rows();++j){
			if(iAdj.at(i,j) == 1){
				dw->line(imagePts.at(0,i)+offsetx, imagePts.at(1,i)+offsety,
						imagePts.at(0,j)+offsetx, imagePts.at(1,j)+offsety);
			}
		}
	}
	dw->unlock();
	dw->update();
	Thread::msleep(delay);
}

void SoftPosit::visualize(ICLDrawWidget &w,const DynMatrix<double> & imagePts, const DynMatrix<double> &imageAdj,
		const DynMatrix<double> &projWorldPts, const DynMatrix<double> &worldAdj, unsigned int delay){
	w.color(255,0,0,1);
	w.linewidth(2);
	w.lock();
	w.reset();
	float offsetx=w.size().rwidth()/2.0;
	float offsety=w.size().rheight()/2.0;
	for(unsigned int i=0;i<worldAdj.cols();++i){
		for(unsigned int j=0;j<worldAdj.rows();++j){
			if(worldAdj.at(i,j)==1){
				w.line(projWorldPts.at(0,i)+offsetx, projWorldPts.at(1,i)+offsety,
						projWorldPts.at(0,j)+offsetx, projWorldPts.at(1,j)+offsety);
			}
		}
	}
	w.color(0,0,255,255);
	for(unsigned int i=0;i<imageAdj.cols();++i){
		for(unsigned int j=0;j<imageAdj.rows();++j){
			if(imageAdj.at(i,j) == 1){
				w.line(imagePts.at(0,i)+offsetx, imagePts.at(1,i)+offsety,
						imagePts.at(0,j)+offsetx, imagePts.at(1,j)+offsety);
			}
		}
	}
	w.unlock();
	w.update();
	Thread::msleep(delay);
}
#endif
}
