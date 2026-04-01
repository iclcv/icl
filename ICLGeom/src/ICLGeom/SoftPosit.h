// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christian Groszewski, Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLMath/DynMatrix.h>
#include <ICLMath/FixedVector.h>
#include <ICLUtils/Macros.h>
#include <ICLUtils/Thread.h>
#include <ICLUtils/Point32f.h>
#include <ICLUtils/BasicTypes.h>

#ifdef ICL_HAVE_QT
#include<ICLQt/DrawWidget.h>
#endif

#ifdef WIN32
  #undef max
#endif

namespace icl{
  namespace geom{
    class ICLGeom_API SoftPosit{
      private:
      //M
      unsigned int nbWorldPts;
      //N
      unsigned int nbImagePts;

      math::DynMatrix<icl64f> centeredImage;

      math::DynMatrix<icl64f> distMat;

      math::DynMatrix<icl64f> assignMat;

      double beta;

      double betaFinal;

      static const double betaUpdate;// not allowed in clang = 1.05;

      static const double betaZero;;// not allowed in clang = 0.0004;
#ifdef ICL_HAVE_QT
      qt::ICLDrawWidget *dw;
#endif
      math::DynMatrix<icl64f> iAdj;
      math::DynMatrix<icl64f> wAdj;

      //expected or random pose
      //math::DynMatrix<icl64f> Q1;
      //expected or random pose
      //math::DynMatrix<icl64f> Q2;
      //squared distances
      math::DynMatrix<icl64f> d;

      math::DynMatrix<icl64f> w;
      //object points
      std::vector<math::DynMatrix<icl64f> > P;
      //image points
      std::vector<math::DynMatrix<icl64f> > p;

      math::DynMatrix<icl64f> L;
      math::DynMatrix<icl64f> invL;

      math::DynMatrix<icl64f> U;
      math::DynMatrix<icl64f> s;
      math::DynMatrix<icl64f> V;
      math::DynMatrix<icl64f> svdResult;

      math::DynMatrix<icl64f> eye2_2;

      math::DynMatrix<icl64f> r1T;
      math::DynMatrix<icl64f> r2T;
      math::DynMatrix<icl64f> r3T;

      math::DynMatrix<icl64f> projectedU;
      math::DynMatrix<icl64f> projectedV;
      math::DynMatrix<icl64f> replicatedProjectedU;
      math::DynMatrix<icl64f> replicatedProjectedV;

      math::DynMatrix<icl64f> col1;
      math::DynMatrix<icl64f> wkxj;
      math::DynMatrix<icl64f> col2;
      math::DynMatrix<icl64f> wkyj;

      math::DynMatrix<icl64f> pts2d;

      math::DynMatrix<icl64f> summedByColAssign;


      math::DynMatrix<icl64f>  weightedUi;
      math::DynMatrix<icl64f>  weightedVi;

      math::DynMatrix<icl64f> R1;
      math::DynMatrix<icl64f> R2;
      math::DynMatrix<icl64f> R3;

      math::DynMatrix<icl64f> ROT;

      math::DynMatrix<icl64f> T;
      double Tz;
      double Tx;
      double Ty;

      double sumNonslack;

      double sum;

      double alpha;

      bool draw;

      math::DynMatrix<icl64f>& cross(math::DynMatrix<icl64f> &x, math::DynMatrix<icl64f> &y, math::DynMatrix<icl64f> &r);

      void maxPosRatio(math::DynMatrix<icl64f> &assignMat, math::DynMatrix<icl64f> &pos, math::DynMatrix<icl64f> &ratios);

      math::DynMatrix<icl64f> &sinkhornImp(math::DynMatrix<icl64f> &M);

      double cond(math::DynMatrix<icl64f> &A);

      double max(math::DynMatrix<icl64f> s);


      public:
      SoftPosit();

      ~SoftPosit();

      void init();

      math::DynMatrix<icl64f> getRotationMat(){
        return ROT;
      }

      math::DynMatrix<icl64f> getTranslation(){
        return T;
      }

      //unused
      int numMatches(math::DynMatrix<icl64f> &assignMat);

      void softPosit(math::DynMatrix<icl64f> imagePts, math::DynMatrix<icl64f> worldPts, double beta0, int noiseStd,	math::DynMatrix<icl64f> initRot,
                     math::DynMatrix<icl64f> initTrans, double focalLength, math::DynMatrix<icl64f> center = math::DynMatrix<icl64f>(2,0), bool draw = true);
#ifdef ICL_HAVE_QT
      void softPosit(math::DynMatrix<icl64f> imagePts, math::DynMatrix<icl64f> imageAdj, math::DynMatrix<icl64f> worldPts,
                     math::DynMatrix<icl64f> worldAdj, double beta0, int noiseStd,	math::DynMatrix<icl64f> initRot,
                     math::DynMatrix<icl64f> initTrans, double focalLength, qt::ICLDrawWidget &w,
                     math::DynMatrix<icl64f> center = math::DynMatrix<icl64f>(2,0), bool draw = true);
#endif
      void softPosit(std::vector<utils::Point32f> imagePts, std::vector<math::FixedColVector<double,3> > worldPts,
                     double beta0, int noiseStd,	math::DynMatrix<icl64f> initRot, math::DynMatrix<icl64f> initTrans,
                     double focalLength, math::DynMatrix<icl64f> center = math::DynMatrix<icl64f>(2,0));
#ifdef ICL_HAVE_QT
      void softPosit(std::vector<utils::Point32f> imagePts, math::DynMatrix<icl64f> imageAdj, std::vector<math::FixedColVector<double,3> > worldPts,
                     math::DynMatrix<icl64f> worldAdj, double beta0, int noiseStd,	math::DynMatrix<icl64f> initRot,
                     math::DynMatrix<icl64f> initTrans, double focalLength, qt::ICLDrawWidget &w,
                     math::DynMatrix<icl64f> center = math::DynMatrix<icl64f>(2,0), bool draw=true);
#endif
      void proj3dto2d(math::DynMatrix<icl64f> pts3d, math::DynMatrix<icl64f> &rot, math::DynMatrix<icl64f> &trans,
                      double flength, int objdim, math::DynMatrix<icl64f> &center, math::DynMatrix<icl64f> &pts2d);

      bool isNullMatrix(const math::DynMatrix<icl64f> &M){
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

#ifdef ICL_HAVE_QT
      void visualize(const math::DynMatrix<icl64f> & imagePts, const math::DynMatrix<icl64f> &projWorldPts, unsigned int delay=200);

      void visualize(qt::ICLDrawWidget &w,const math::DynMatrix<icl64f> & imagePts, const math::DynMatrix<icl64f> &imageAdj,
                     const math::DynMatrix<icl64f> &projWorldPts, const math::DynMatrix<icl64f> &worldAdj, unsigned int delay=200);
#endif
    };
  } // namespace geom
}
