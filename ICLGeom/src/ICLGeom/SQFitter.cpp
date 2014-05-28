/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/SQFitter.cpp                       **
** Module : ICLGeom                                                **
** Authors: Sergius Gaulik                                         **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLGeom/SQFitter.h>

#include <ICLUtils/Macros.h>
#include <ICLUtils/Function.h>
#include <ICLMath/MathFunctions.h>
#include <ICLGeom/GeomDefs.h>
#include <ICLMath/FixedMatrix.h>

typedef icl::math::LevenbergMarquardtFitter<float> LM;

namespace icl{
  namespace geom{ 

    icl::math::FixedMatrix<float,4,4> SQParams::getTransformationMatrix() const {
      return icl::math::create_hom_4x4<float>(euler[0], euler[1], euler[2],
                                              pos[0], pos[1], pos[2]);
    }

    // Mx already contains the multiplication with the inversed homographic matrix
    LM::Matrix SQFitter::shapeError::operator()(const LM::Params &p, const LM::Matrix &Mx){
      const float e1 = p[0];
      const float e2 = p[1];

      LM::Matrix My(Mx.cols(), 1);

      const float *it0 = Mx.row_begin(0);
      const float *it1 = Mx.row_begin(1);
      const float *it2 = Mx.row_begin(2);
      float *itY = My.begin();
      float *itEnd = My.end();

      for (;itY != itEnd; it0++, it1++, it2++, itY++) {
        *itY = pow(pow(pow(*it0, 2.0f/e2)
                     + pow(*it1, 2.0f/e2), e2/e1)
                 + pow(*it2, 2.0f/e1), e1) - 1.0f;
      }

      return My;
    }

    LM::Matrix SQFitter::eulerPosError::operator()(const LM::Params &p, const LM::Matrix &Mx){
      const float a1 = size[0];
      const float a2 = size[1];
      const float a3 = size[2];
      const float e1 = shape[0];
      const float e2 = shape[1];

      LM::Matrix My(Mx.cols(), 1);
      LM::Matrix A(Mx.cols(), 4, 1.0);
      std::copy(Mx.begin(), Mx.end(), A.begin());

      A = math::create_hom_4x4<float>(p[0], p[1], p[2],
                                      p[3], p[4], p[5]).inv().dyn() * A;

      float *it0 = A.row_begin(0);
      float *it1 = A.row_begin(1);
      float *it2 = A.row_begin(2);
      float *itY = My.begin();
      float *itEnd = My.end();

      for (;itY != itEnd; it0++, it1++, it2++, itY++) {
        *itY = pow(pow(pow(fabs(*it0)/a1, 2.0f/e2)
                     + pow(fabs(*it1)/a2, 2.0f/e2), e2/e1)
                 + pow(fabs(*it2)/a3, 2.0f/e1), e1) - 1.0f;
      }

      return My;
    }


    SQFitter::SQFitter(icl::utils::SmartPtr<Vec> camCenter) : camCenter(camCenter) {
      // get the error functions
      sErrorFunc = utils::function(sError, &shapeError::operator());
      ePErrorFunc = utils::function(ePError, &eulerPosError::operator());

      // initialize Levenberg-Marquardt
      shapeLM = LM(sErrorFunc,1,std::vector<LM::JacobianMat>(),1.e-3,200,1.e-6,10,1.e-6,1.e-6,"svd");
      eulerPosLM = LM(ePErrorFunc,1,std::vector<LM::JacobianMat>(),1.e-3,200,1.e-6,100,1.e-6,1.e-6,"svd");
    }

    void SQFitter::preProcess(LM::Matrix &Mx, Vec3 &viewDir, const std::string& sShapePreference,
                              icl::math::FixedMatrix<float,3,3> &R,
                              Vec3 &center, Vec3 &size, Vec3 &origin, float &scale) {
      int cols = Mx.cols();

      // point cloud center
      center = Vec3(math::mean(Mx.row_begin(0), Mx.row_end(0)),
                    math::mean(Mx.row_begin(1), Mx.row_end(1)),
                    math::mean(Mx.row_begin(2), Mx.row_end(2)));

      for (int i = 0; i < cols; ++i) {
        Mx(i,0) -= center[0];
        Mx(i,1) -= center[1];
        Mx(i,2) -= center[2];
      }

      // matrix of central moments
      LM::Matrix M(3,3,0.0f);

      for (int i = 0; i < cols; ++i) {
        M(0,0) += Mx(i,0)*Mx(i,0);
        M(1,0) += Mx(i,0)*Mx(i,1);
        M(2,0) += Mx(i,0)*Mx(i,2);
        M(1,1) += Mx(i,1)*Mx(i,1);
        M(2,1) += Mx(i,1)*Mx(i,2);
        M(2,2) += Mx(i,2)*Mx(i,2);
      }

      M(0,1) = M(1,0);
      M(0,2) = M(2,0);
      M(1,2) = M(2,1);

      M /= (float)cols;


      // eigen vectors + eigen values
      LM::Matrix eVec, eVal;
      M.eigen(eVec, eVal);

      for (icl::math::DynMatrix<float>::iterator it = eVal.begin(); it != eVal.end(); ++it)
        *it = sqrt(fabs(*it));


      // get bounding box
      LM::Matrix D = eVec.inv()*Mx;
      Vec3 minV(D(0,0), D(0,1), D(0,2));
      Vec3 maxV(minV);

      for (unsigned int i = 1; i < D.cols(); ++i) {
        if (D(i,0) < minV[0]) minV[0] = D(i,0);
        else if (D(i,0) > maxV[0]) maxV[0] = D(i,0);
        if (D(i,1) < minV[1]) minV[1] = D(i,1);
        else if (D(i,1) > maxV[1]) maxV[1] = D(i,1);
        if (D(i,2) < minV[2]) minV[2] = D(i,2);
        else if (D(i,2) > maxV[2]) maxV[2] = D(i,2);
      }

      size = maxV - minV;


      // scale everything
      scale = 1.0f/size.length();

      Mx *= scale;
      eVal *= scale;
      minV *= scale;
      maxV *= scale;
      size *= scale;
      size /= 2.0f;


      // get origin
      R = icl::math::FixedMatrix<float,3,3>(eVec.data());
      origin = R * ((minV + maxV) * 0.5f);
    }

    LM::Result SQFitter::fitShape(int i, icl::math::FixedMatrix<float,3,3> &R, LM::Matrix &xyzM, SQParams &params, Vec3 &size, Vec3 &euler) {
      Vec3 pos = params.pos;


      icl::math::FixedMatrix<float,3,3> sR;

      // switch axis if needed
      if (i == 0) {
        size = params.size;
        sR = R;
      } else if (i == 1) {
        size[0] = params.size[0];
        size[1] = params.size[2];
        size[2] = params.size[1];
        sR = icl::math::FixedMatrix<float,3,3>(R(0,0), R(2,0), R(1,0),
                                               R(0,1), R(2,1), R(1,1),
                                               R(0,2), R(2,2), R(1,2));
      } else {
        size[0] = params.size[1];
        size[1] = params.size[2];
        size[2] = params.size[0];
        sR = icl::math::FixedMatrix<float,3,3>(R(1,0), R(2,0), R(0,0),
                                               R(1,1), R(2,1), R(0,1),
                                               R(1,2), R(2,2), R(0,2));
      }

      // recalculate z-axis for the correct sign
      sR(2,0) = sR(0,1)*sR(1,2) - sR(0,2)*sR(1,1);
      sR(2,1) = sR(0,2)*sR(1,0) - sR(0,0)*sR(1,2);
      sR(2,2) = sR(0,0)*sR(1,1) - sR(0,1)*sR(1,0);


      // calculate euler angles
      euler = icl::math::extract_euler_angles(sR);


      // first guess for the shape is (1,1)
      float pTmp[2];
      pTmp[0] = 1.0f;
      pTmp[1] = 1.0f;
      LM::Params p(2,pTmp);


      // the following calculation are always the same for the error function
      LM::Matrix A(xyzM.cols(), 4, 1.0);
      std::copy(xyzM.begin(), xyzM.end(), A.begin());

      A = icl::math::create_hom_4x4<float>(euler[0], euler[1], euler[2],
                                           pos[0], pos[1], pos[2]).inv().dyn() * A;

      float a0 = 1.0/size[0];
      float a1 = 1.0/size[1];
      float a2 = 1.0/size[2];
      for (float *it = A.row_begin(0); it != A.row_end(0); ++it) *it = fabs(*it)*a0;
      for (float *it = A.row_begin(1); it != A.row_end(1); ++it) *it = fabs(*it)*a1;
      for (float *it = A.row_begin(2); it != A.row_end(2); ++it) *it = fabs(*it)*a2;

      // try to improve the shape paremeter using Levenberg-Marquardt
      LM::Result result = shapeLM.fit(A, LM::Matrix(1,xyzM.cols(),0.0f), p);

      return result;
    }

    void SQFitter::fitSQ(LM::Matrix &Mx, Vec3 &viewDir, const std::string& sShapePreference) {
      icl::math::FixedMatrix<float,3,3> R;
      Vec3 center, size, origin;
      float scale;

      // prepare some data
      preProcess(Mx, viewDir, sShapePreference, R, center, size, origin, scale);


      SQParams tmpParams, bestParams;
      LM::Result bestResult, curResult;
      Vec3 tmpSize, tmpEuler;

      tmpParams.size = size;
      tmpParams.pos  = origin;
      tmpParams.shape[0] = 1.0f;
      tmpParams.shape[1] = 1.0f;


      // calculate shape parameters with diffrent axis and choose the best solution
      bestResult = fitShape(0,R,Mx,tmpParams,bestParams.size,bestParams.euler);
      bestParams.shape[0] = bestResult.params[0];
      bestParams.shape[1] = bestResult.params[1];

      curResult = fitShape(1,R,Mx,tmpParams,tmpSize,tmpEuler);

      if (curResult.error < bestResult.error) {
        bestResult = curResult;
        bestParams.size = tmpSize;
        bestParams.euler = tmpEuler;
        bestParams.shape[0] = bestResult.params[0];
        bestParams.shape[1] = bestResult.params[1];
      }

      curResult = fitShape(2,R,Mx,tmpParams,tmpSize,tmpEuler);

      if (curResult.error < bestResult.error) {
        bestResult = curResult;
        bestParams.size = tmpSize;
        bestParams.euler = tmpEuler;
        bestParams.shape[0] = bestResult.params[0];
        bestParams.shape[1] = bestResult.params[1];
      }


      float pTmp[6];
      pTmp[0] = bestParams.euler[0];
      pTmp[1] = bestParams.euler[1];
      pTmp[2] = bestParams.euler[2];
      pTmp[3] = tmpParams.pos[0];
      pTmp[4] = tmpParams.pos[1];
      pTmp[5] = tmpParams.pos[2];
      LM::Params p(6,pTmp);


      // calculate position and euler parameters
      ePError.size  = bestParams.size;
      ePError.shape = bestParams.shape;
      LM::Result result = eulerPosLM.fit(Mx, LM::Matrix(1,Mx.cols(),0.0f), p);


      scale = 1.0f/scale;

      // scale the size and position back to normal
      bestParams.size  *= scale;
      result.params[3] *= scale;
      result.params[4] *= scale;
      result.params[5] *= scale;
      result.params[3] += center[0];
      result.params[4] += center[1];
      result.params[5] += center[2];


      // set the found paremeters
      params.size     = bestParams.size;
      params.shape    = bestParams.shape;
      params.euler[0] = result.params[0];
      params.euler[1] = result.params[1];
      params.euler[2] = result.params[2];
      params.pos[0]   = result.params[3];
      params.pos[1]   = result.params[4];
      params.pos[2]   = result.params[5];
    }

    bool SQFitter::fit(PointCloudObject& pcObj, 
                 const std::string& sShapePreference,
                 size_t maxN) {
      if (maxN == 0) maxN = pcObj.getDim();
      if (maxN < 11) {
        ERROR_LOG("SQFitter: too few points: " << maxN);
        return false;
      }

      maxN = iclMin(int(maxN),250);

      Vec3 viewDir(0,0,0);

      if (camCenter) {
        Vec c = *camCenter;
        const core::DataSegment<float,4> xyz = pcObj.selectXYZH();
        Vec cpc = std::accumulate(&xyz[0],&xyz[0]+xyz.getDim(), Vec(0,0,0,0));
        cpc *= 1./xyz.getDim();

        viewDir = (cpc - c).part<0,0,1,3>();
        viewDir.normalize();
      }

      size_t N = pcObj.getDim();
      LM::Matrix Mx(iclMin(N,maxN),3);
      core::DataSegment<float,3> xyz = pcObj.selectXYZ();

      // get points from the point cloud
      for (size_t i=0, n=0; i < N; ++i) {
        if (i * maxN / N >= n) {
          Mx(n,0) = xyz[i][0];
          Mx(n,1) = xyz[i][1];
          Mx(n,2) = xyz[i][2];
          ++n;
        }
      }

      // try to fit a superquadric in the given point cloud
      fitSQ(Mx, viewDir, sShapePreference);

      return true;
    }

    void SQFitter::setCameraCenter(const Vec &c){
      camCenter = new Vec(c);
    }

  } // namespace geom
}
