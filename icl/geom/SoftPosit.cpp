// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christian Groszewski, Christof Elbrechter

#include <icl/geom/SoftPosit.h>



using namespace icl::utils;
using namespace icl::math;
#ifdef ICL_HAVE_QT
using namespace icl::qt;
#endif

namespace icl::geom {
  const double SoftPosit::betaUpdate = 1.05;

  const double SoftPosit::betaZero = 0.0004;

#ifdef ICL_HAVE_QT
  SoftPosit::SoftPosit():dw(0){
    ROT.setBounds(3,3);
    T.setBounds(1,3);
    R1.setBounds(1,3);
    R2.setBounds(1,3);
    R3.setBounds(1,3);
    eye2_2.setBounds(2,2);
    eye2_2(0, 0) = 1.0;
    eye2_2(1, 1) = 1.0;
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
    eye2_2(0, 0) = 1.0;
    eye2_2(1, 1) = 1.0;
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

  void SoftPosit::softPosit(DynMatrix<icl64f> imagePts, DynMatrix<icl64f> worldPts, double beta0, int noiseStd,	DynMatrix<icl64f> initRot,
                            DynMatrix<icl64f> initTrans, double focalLength, DynMatrix<icl64f> center, bool draw){
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
    distMat=DynMatrix<icl64f>(nbWorldPts,nbImagePts);
    //das ist dann gamma
    double max = iclMax(nbImagePts,nbWorldPts);
    double scale = 1.0/(max + 1);

    centeredImage.setBounds(imagePts.cols(),imagePts.rows());
    for(unsigned int i=0;i<imagePts.rows();++i){
      centeredImage(i, 0) =(imagePts(i, 0)-center(0, 0))/focalLength;
      centeredImage(i, 1) =(imagePts(i, 1)-center(0, 1))/focalLength;
    }
    DynMatrix<icl64f> imageOnes(1,nbImagePts,1);
    DynMatrix<icl64f> homogeneousWorldPts(4,nbWorldPts) ;
    for(unsigned int i=0;i<nbWorldPts;++i){
      for(unsigned int j=0;j<3;++j){
        homogeneousWorldPts(i, j) = worldPts(i, j);
      }
      homogeneousWorldPts(i, 3) = 1.0;
    }
    //Initial rotation and translation as passed into this function.
    ROT = initRot;
    T = initTrans;

    //Initialize the depths of all world points based on initial pose.

    DynMatrix<icl64f> temp(1,4,1);
    for(unsigned int i=0;i<3;++i){
      temp(i, 0)=ROT(2, i)/T(2, 0);
    }
    DynMatrix<icl64f> wk = homogeneousWorldPts * temp;

#ifdef ICL_HAVE_QT
    //DynMatrix<icl64f> projWorldPts = proj3dto2d(worldPts, rot, trans, focalLength, 1, center);
    if(draw){
      proj3dto2d(worldPts, ROT, T, focalLength,1,center,pts2d);
      //visualize(w,imagePts, imageAdj,	pts2d, worldAdj);
      visualize(imagePts, pts2d);
    }
#endif
    //First two rows of the camera matrices (for both perspective and SOP).  Note:
    //the scale factor is s = f/Tz = 1/Tz since f = 1.  These are column 4-vectors.
    double t1[] = {ROT(0, 0)/T(2, 0),ROT(0, 1)/T(2, 0),ROT(0, 2)/T(2, 0), T(0, 0)/T(2, 0)};
    double t2[] = {ROT(1, 0)/T(2, 0),ROT(1, 1)/T(2, 0),ROT(1, 2)/T(2, 0), T(1, 0)/T(2, 0)};
    r1T = DynMatrix<icl64f>(1,4,t1);
    r2T = DynMatrix<icl64f>(1,4,t2);
    r3T.setBounds(1,4);
    int betaCount = 0;
    int poseConverged = 0;
    int assignConverged = 0;
    //	int foundPose = 0;
    beta = beta0;
    DynMatrix<icl64f> r1Tr2T(2,3);
    assignMat = DynMatrix<icl64f>(nbWorldPts+1,nbImagePts+1,1+epsilon0);

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
      DynMatrix<icl64f> temp1 = replicatedProjectedU - wkxj;
      DynMatrix<icl64f> temp2 = replicatedProjectedV - wkyj;
      for(unsigned int i=0;i<nbWorldPts;++i){
        for(unsigned j=0;j<nbImagePts;++j){
          distMat(j, i) = focalLength*focalLength*(temp1(j, i)*temp1(j, i)+temp2(j, i)*temp2(j, i));
        }
      }
      for(unsigned int i=0;i<nbWorldPts;++i){
        for(unsigned int j=0;j<nbImagePts;++j){
          assignMat(j, i) = scale * ( std::exp( -beta*( distMat(j, i) - alpha ) ) );
        }
      }
      //assignMat(1:nbImagePts+1,nbWorldPts+1) = scale;
      for(unsigned int i=0;i<nbImagePts+1;++i){
        assignMat(i, nbWorldPts) = scale;
      }
      //assignMat(nbImagePts+1,1:nbWorldPts+1) = scale;
      for(unsigned int i=0;i<nbWorldPts+1;++i){
        assignMat(nbImagePts, i) = scale;
      }
      sinkhornImp(assignMat);

      //int numMatchPts = numMatches(assignMat);

      //sumNonslack = sum(sum(assignMat(1:nbImagePts,1:nbWorldPts)));
      sumNonslack=0.0;
      for(unsigned int i=0;i<nbWorldPts;++i){
        for(unsigned int j=0;j<nbImagePts;++j){
          sumNonslack += assignMat(j, i);
        }
      }
      //summedByColAssign = sum(assignMat(1:nbImagePts, 1:nbWorldPts), 1);
      summedByColAssign = DynMatrix<icl64f>(nbWorldPts,1);
      for(unsigned int i=0;i<nbWorldPts;++i){
        for(unsigned int j=0;j<nbImagePts;++j){
          summedByColAssign(0, i) += assignMat(j, i);
        }
      }

      //TODO optimize
      L = DynMatrix<icl64f>(4,4);
      DynMatrix<icl64f> temp11(1,4);
      DynMatrix<icl64f> temp22(4,1);
      DynMatrix<icl64f> temp33(4,4);
      DynMatrix<icl64f> temp44(4,4);
      for (unsigned int k = 0;k<nbWorldPts;k++){
        for(int i=0;i<4;++i){
          temp11(i, 0) = homogeneousWorldPts(k, i);
          temp22(0, i) = homogeneousWorldPts(k, i);
        }
        //sumSkSkT = sumSkSkT + summedByColAssign(1, k) * homogeneousWorldPts(k,:)' * homogeneousWorldPts(k,:);
        temp33 = temp11 * temp22;
        temp33.mult(summedByColAssign(0, k),temp44);
        L = L + temp44;
      }
      if (cond(L) > 1e10){
        std::cout << "cond L to small\n";
        return;
      }
      invL = L.inv();

      //poseConverged = 0;
      //TODO optimize
      weightedUi = DynMatrix<icl64f>(1,4);
      weightedVi = DynMatrix<icl64f>(1,4);

      DynMatrix<icl64f>  temp55(1,4);
      DynMatrix<icl64f>  temp66(1,4);
      for(unsigned int j = 0;j<nbImagePts;++j){
        for(unsigned int k = 0;k<nbWorldPts;++k){
          for(int i=0;i<4;++i){
            temp55(i, 0) = homogeneousWorldPts(k, i);
          }
          temp55.mult(assignMat(j, k) * wk(0, k) * centeredImage(j, 0), temp66);
          weightedUi = weightedUi + temp66;
          temp55.mult(assignMat(j, k) * wk(0, k) * centeredImage(j, 1), temp66);
          weightedVi = weightedVi + temp66;
        }
      }
      // % Compute the pose vectors. M = s(R1,Tx) and N = s(R2,Ty) where the
      //% scale factor is s = f/Tz, and f = 1.  These are column 4-vectors.
      r1T = invL * weightedUi;
      r2T = invL * weightedVi;

      for(unsigned int i=0;i<3;++i){
        r1Tr2T(i, 0) = r1T(i, 0);
        r1Tr2T(i, 1) = r2T(i, 0);
      }

      if (1) {//calculation of R and T.
        r1Tr2T.svd(U,s,V);
        svdResult = U * eye2_2 * V.transp();
        for(unsigned int i=0;i<3;++i){
          R1(i, 0) = svdResult(i, 0);
          R2(i, 0) = svdResult(i, 1);
        }
        cross(R1,R2,R3);
        Tz = 2 / (s(0, 0) + s(1, 0));
        Tx = r1T(3, 0) * Tz;
        Ty = r2T(3, 0) * Tz;
        r3T(0, 0)=R3(0, 0);
        r3T(1, 0)=R3(1, 0);
        r3T(2, 0)=R3(2, 0);
        r3T(3, 0)=Tz;
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
      r1T(0, 0) = R1(0, 0)/Tz;
      r1T(1, 0) = R1(1, 0)/Tz;
      r1T(2, 0) = R1(2, 0)/Tz;
      r1T(3, 0) = Tx/Tz;
      r2T(0, 0) = R2(0, 0)/Tz;
      r2T(1, 0) = R2(1, 0)/Tz;
      r2T(2, 0) = R2(2, 0)/Tz;
      r2T(3, 0) = Ty/Tz;
      //TODO
      DynMatrix<icl64f> temp001(1,3);
      r3T.mult(1/Tz,temp001);
      wk = homogeneousWorldPts * temp001;
      //delta = sqrt(sum(sum(assignMat(1:nbImagePts,1:nbWorldPts) .* distMat))/nbWorldPts);
      temp001.setBounds(nbWorldPts,nbImagePts,false);
      for(unsigned int i=0;i<nbWorldPts;++i){
        for(unsigned int j=0;j<nbImagePts;++j){
          temp001(j, i) = assignMat(j, i)*distMat(j, i);
        }
      }
      sum = 0.0;
      for(unsigned int i=0;i<temp001.cols();++i){
        for(unsigned int j=0;j<temp001.rows();++j){
          sum += temp001(j, i);
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

      T(0, 0) = Tx;
      T(1, 0) = Ty;
      T(2, 0) = Tz;

      for(unsigned int i=0;i<3;++i){
        ROT(0, i) = R1(i, 0);
        ROT(1, i) = R2(i, 0);
        ROT(2, i) = R3(i, 0);
      }
      //	if(delta < maxDelta && betaCount > minBetaCount)
      //	foundPose = 1;
      //else
      //	foundPose = 0;
#ifdef ICL_HAVE_QT
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
#ifdef ICL_HAVE_QT
  void SoftPosit::softPosit(DynMatrix<icl64f> imagePts, DynMatrix<icl64f> imageAdj, DynMatrix<icl64f> worldPts,
                            DynMatrix<icl64f> worldAdj, double beta0, int noiseStd,	DynMatrix<icl64f> initRot,
                            DynMatrix<icl64f> initTrans, double focalLength, ICLDrawWidget &w,
                            DynMatrix<icl64f> center, bool draw){
    dw = &w;
    iAdj = imageAdj;
    wAdj = worldAdj;
    softPosit(imagePts, worldPts, beta0, noiseStd, initRot, initTrans, focalLength, center);
  }
#endif
  void SoftPosit::softPosit(std::vector<Point32f> imagePts, std::vector<FixedColVector<double,3> > worldPts,
                            double beta0, int noiseStd,	DynMatrix<icl64f> initRot, DynMatrix<icl64f> initTrans,
                            double focalLength, DynMatrix<icl64f> center){
    DynMatrix<icl64f> imagePt(2,imagePts.size());
    for(unsigned int i=0; i<imagePts.size();++i){
      imagePt(i, 0) = imagePts.at(i).x;
      imagePt(i, 1) = imagePts.at(i).y;
    }
    DynMatrix<icl64f> worldPt(3,worldPts.size());
    for(unsigned int i=0; i<worldPts.size();++i){
      worldPt(i, 0) = worldPts.at(i)(0, 0);
      worldPt(i, 1) = worldPts.at(i)(1, 0);
      worldPt(i, 2) = worldPts.at(i)(2, 0);
    }

    softPosit(imagePt, worldPt, beta0, noiseStd, initRot, initTrans, focalLength, center, draw);
  }
#ifdef ICL_HAVE_QT
  void SoftPosit::softPosit(std::vector<Point32f> imagePts, DynMatrix<icl64f> imageAdj, std::vector<FixedColVector<double,3> > worldPts,
                            DynMatrix<icl64f> worldAdj, double beta0, int noiseStd,	DynMatrix<icl64f> initRot,
                            DynMatrix<icl64f> initTrans, double focalLength, ICLDrawWidget &w, DynMatrix<icl64f> center,bool draw){

    DynMatrix<icl64f> imagePt(2,imagePts.size());
    for(unsigned int i=0; i<imagePts.size();++i){
      imagePt(i, 0) = imagePts.at(i).x;
      imagePt(i, 1) = imagePts.at(i).y;
    }
    DynMatrix<icl64f> worldPt(3,worldPts.size());
    for(unsigned int i=0; i<worldPts.size();++i){
      worldPt(i, 0) = worldPts.at(i)(0, 0);
      worldPt(i, 1) = worldPts.at(i)(1, 0);
      worldPt(i, 2) = worldPts.at(i)(2, 0);
    }

    softPosit(imagePt, imageAdj, worldPt, worldAdj, beta0, noiseStd, initRot, initTrans, focalLength, w, center, draw);
  }
#endif

  DynMatrix<icl64f>& SoftPosit::cross(DynMatrix<icl64f> &x, DynMatrix<icl64f> &y, DynMatrix<icl64f> &r){
    if(x.cols()==1 && y.cols()==1 && x.rows()==3 && y.rows()==3){
      r(0, 0) = x(1, 0)*y(2, 0)-x(2, 0)*y(1, 0);
      r(1, 0) = x(2, 0)*y(0, 0)-x(0, 0)*y(2, 0);
      r(2, 0) = x(0, 0)*y(1, 0)-x(1, 0)*y(0, 0);
    }
    return r;
  }

  void SoftPosit::proj3dto2d(DynMatrix<icl64f> pts3d, DynMatrix<icl64f> &rot, DynMatrix<icl64f> &trans,
                             double flength, int objdim, DynMatrix<icl64f> &center, DynMatrix<icl64f> &pts2d){
    //3D point matrix must be 3xN.
    if (objdim == 1){
      pts3d = pts3d.transp();
    }
    //number of 3D points.
    unsigned int numpts = pts3d.cols();
    DynMatrix<icl64f> newtrans(numpts,3);
    for(unsigned int i=0;i<numpts;++i){
      newtrans(0, i) = trans(0, 0);
      newtrans(1, i) = trans(1, 0);
      newtrans(2, i) = trans(2, 0);
    }
    DynMatrix<icl64f> campts = rot*pts3d+newtrans;
    for(unsigned int i=0;i<campts.cols();++i){
      if(campts(2, i)<1e-20){
        campts(2, i) = 1e-20;
      }
    }
    pts2d.setBounds(campts.cols(),2);
    for(unsigned int i=0;i<campts.cols();++i){
      pts2d(0, i) = flength * campts(0, i)*(1/campts(2, i));
      pts2d(1, i) = flength * campts(1, i)*(1/campts(2, i));
    }
    DynMatrix<icl64f> cent(numpts,2);
    for(unsigned int i=0;i<numpts;++i){
      cent(0, i) = center(0, 0);
      cent(1, i) = center(0, 1);
    }
    pts2d = pts2d +cent;
    if (objdim == 1){
      pts2d = pts2d.transp();
    }
  }

  void SoftPosit::maxPosRatio(DynMatrix<icl64f> &assignMat, DynMatrix<icl64f> &pos, DynMatrix<icl64f> &ratios){
    unsigned int nrows = assignMat.rows()-1;
    double vmax =0.0;
    unsigned int imax=0;
    bool isMaxInRow = true;
    double cr = 0.0;
    double rr = 0.0;
    for(unsigned int k = 0;k<assignMat.cols()-1;++k){
      vmax = assignMat(0, k);
      imax = 0;
      for(unsigned int i=1;i<assignMat.rows();++i){
        if(vmax < assignMat(i, k)){
          vmax = assignMat(i, k);
          imax = i;
        }
      }
      if (imax == nrows)
        continue;
      isMaxInRow = true;
      for(unsigned int i=0;i<assignMat.cols();++i){
        if(vmax < assignMat(imax, i) && i != k)
          isMaxInRow = false;
      }
      if(isMaxInRow){
        pos.setBounds(2,pos.rows()+1);
        pos(pos.rows()-1, 0) = imax;
        pos(pos.rows()-1, 1) = k;
        rr = assignMat(imax, assignMat.cols()-1)/assignMat(imax, k);
        cr = assignMat(assignMat.rows()-1, k)/assignMat(imax, k);
        ratios.setBounds(2,ratios.rows()+1);
        ratios(ratios.rows()-1, 0) = rr;
        ratios(ratios.rows()-1, 1) = cr;
      }
    }
  }

  //TODO maybe implement normal/standard sinkhorn
  DynMatrix<icl64f> &SoftPosit::sinkhornImp(DynMatrix<icl64f> &M){
    int iMaxIterSinkhorn=60;
    double fEpsilon2 = 0.001;
    int iNumSinkIter = 0;
    unsigned int nbRows = M.rows();
    unsigned int nbCols = M.cols();
    double fMdiffSum = fEpsilon2 + 1;
    DynMatrix<icl64f> ratios;
    DynMatrix<icl64f> posmax;
    maxPosRatio(M,posmax,ratios);
    DynMatrix<icl64f> Mprev;
    DynMatrix<icl64f> McolSums;
    DynMatrix<icl64f> MrowSums;
    DynMatrix<icl64f> ones(1,M.cols(),1.0);
    DynMatrix<icl64f> MrowSumsRep;
    DynMatrix<icl64f> McolSumsRep;
    while(std::abs(fMdiffSum) > fEpsilon2 && iNumSinkIter < iMaxIterSinkhorn){
      McolSums = DynMatrix<icl64f>(M.cols(),1);
      MrowSums = DynMatrix<icl64f>(1,M.rows());
      Mprev = M;
      for(unsigned int j=0;j<M.cols();++j)
        for(unsigned int i=0;i<M.rows();++i){
          McolSums(0, j) += M(i, j);
        }

      McolSums(0, nbCols-1) = 1;
      ones.setBounds(1,M.cols(),false,1.0);
      McolSumsRep = ones * McolSums;
      for(unsigned int i=0; i<M.cols();++i){
        for(unsigned j=0; j<M.rows();++j){
          M(j, i) = M(j, i)/McolSumsRep(j, i);
        }
      }
      for(unsigned int i=0;i<posmax.rows();++i){
        M(posmax(i, 0), nbCols-1) = ratios(i, 0)*M(posmax(i, 1), posmax(i, 0));
      }
      for(unsigned int j=0;j<M.rows();++j)
        for(unsigned int i=0;i<M.cols();++i){
          MrowSums(j, 0) += M(j, i);
        }
      MrowSums(nbRows-1, 0) = 1;
      ones.setBounds(nbCols,1,false,1.0);
      MrowSumsRep = MrowSums*ones;
      for(unsigned int i=0; i<M.cols();++i){
        for(unsigned j=0; j<M.rows();++j){
          M(j, i) = M(j, i)/MrowSumsRep(j, i);
        }
      }
      for(unsigned int i=0;i<posmax.rows();++i){
        M(posmax(i, 1), nbRows) = ratios(i, 2)*M(posmax(i, 2), posmax(i, 1));
      }
      iNumSinkIter=iNumSinkIter+1;
      fMdiffSum = 0.0;
      for(unsigned int i=0;i<M.cols();++i){
        for(unsigned int j=0;j<M.rows();++j){
          fMdiffSum += std::abs(M(j, i)-Mprev(j, i));
        }
      }
    }
    return M;
  }

  int SoftPosit::numMatches(DynMatrix<icl64f> &assignMat){
    int num = 0;
    int nrows = assignMat.rows();
    double vmax = 0.0;
    int imax = 0;
    bool isMaxInRow = true;
    for(unsigned int k = 0 ;k<assignMat.cols();++k){
      vmax = assignMat(0, k);
      imax = 0;
      for(unsigned int i=1;i<assignMat.rows();++i){
        if(vmax < assignMat(i, k)){
          vmax = assignMat(i, k);
          imax = i;
        }
      }
      if (imax == nrows){
        continue;
      }
      isMaxInRow = true;
      for(unsigned int i=0;i<assignMat.cols();++i){
        if(vmax < assignMat(imax, i) && i != k)
          isMaxInRow = false;
      }
      if(isMaxInRow){
        num = num + 1;
      }
    }
    return num;
  }

  double SoftPosit::cond(DynMatrix<icl64f> &A){
    DynMatrix<icl64f> U;
    DynMatrix<icl64f> V;
    DynMatrix<icl64f> s;
    A.svd(U,s,V);
    double n1 = max(s);
    (A.inv()).svd(U,s,V);
    double n2 = max(s);
    return n1 * n2;
  }

  double SoftPosit::max(DynMatrix<icl64f> s){
    double max = s(0, 0);
    for(unsigned int i=0;i<s.rows();++i){
      if(max < s(i, 0))
        max = s(i, 0);
    }
    return max;
  }

#ifdef ICL_HAVE_QT

  void SoftPosit::visualize(const DynMatrix<icl64f> & imagePts, const DynMatrix<icl64f> &projWorldPts, unsigned int delay){
    dw->color(255,0,0,1);
    dw->linewidth(2);
    //	dw->lock();
    //dw->reset();
    float offsetx=dw->size().rwidth()/2.0;
    float offsety=dw->size().rheight()/2.0;
    for(unsigned int i=0;i<wAdj.cols();++i){
      for(unsigned int j=0;j<wAdj.rows();++j){
        if(wAdj(j, i)==1){
          dw->line(projWorldPts(i, 0)+offsetx, projWorldPts(i, 1)+offsety,
                   projWorldPts(j, 0)+offsetx, projWorldPts(j, 1)+offsety);
        }
      }
    }
    dw->color(0,0,255,255);
    for(unsigned int i=0;i<iAdj.cols();++i){
      for(unsigned int j=0;j<iAdj.rows();++j){
        if(iAdj(j, i) == 1){
          dw->line(imagePts(i, 0)+offsetx, imagePts(i, 1)+offsety,
                   imagePts(j, 0)+offsetx, imagePts(j, 1)+offsety);
        }
      }
    }
    //dw->unlock();
    dw->render();//update();
    Thread::msleep(delay);
  }

  void SoftPosit::visualize(ICLDrawWidget &w,const DynMatrix<icl64f> & imagePts, const DynMatrix<icl64f> &imageAdj,
                            const DynMatrix<icl64f> &projWorldPts, const DynMatrix<icl64f> &worldAdj, unsigned int delay){
    w.color(255,0,0,1);
    w.linewidth(2);
    //	w.lock();
    //w.reset();
    float offsetx=w.size().rwidth()/2.0;
    float offsety=w.size().rheight()/2.0;
    for(unsigned int i=0;i<worldAdj.cols();++i){
      for(unsigned int j=0;j<worldAdj.rows();++j){
        if(worldAdj(j, i)==1){
          w.line(projWorldPts(i, 0)+offsetx, projWorldPts(i, 1)+offsety,
                 projWorldPts(j, 0)+offsetx, projWorldPts(j, 1)+offsety);
        }
      }
    }
    w.color(0,0,255,255);
    for(unsigned int i=0;i<imageAdj.cols();++i){
      for(unsigned int j=0;j<imageAdj.rows();++j){
        if(imageAdj(j, i) == 1){
          w.line(imagePts(i, 0)+offsetx, imagePts(i, 1)+offsety,
                 imagePts(j, 0)+offsetx, imagePts(j, 1)+offsety);
        }
      }
    }
    //w.unlock();
    w.render();
    Thread::msleep(delay);
  }
#endif
  } // namespace icl::geom