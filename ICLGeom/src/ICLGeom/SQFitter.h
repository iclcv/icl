// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Sergius Gaulik, Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLMath/FixedMatrix.h>
#include <ICLMath/LevenbergMarquardtFitter.h>
#include <ICLGeom/PointCloudObject.h>
#include <vector>

namespace icl{
  namespace geom{

    /// Utility structure, that represents a superquadric
    struct ICLGeom_API SQParams {
      math::Vec3 size;
      math::Vec3 pos;
      math::Vec3 euler;
      std::vector<float> shape;

      icl::math::FixedMatrix<float,4,4> getTransformationMatrix() const;
      SQParams () : shape(2, 1.0) {}
    };

    class ICLGeom_API SQFitter{
    public:
      using LM = icl::math::LevenbergMarquardtFitter<float>; //!< Levenberg-Marquardt

    private:

      /// Structure, that contains the error function for the shape parameters
      struct shapeError {
        LM::Matrix operator()(const LM::Params &p, const LM::Matrix &vx);
      };

      /// Structure, that contains the error function for the euler angles and position parameters
      struct eulerPosError {
        math::Vec3 size;
        std::vector<float> shape;

        LM::Matrix operator()(const LM::Params &p, const LM::Matrix &vx);
      };

    public:
      /// Constructor
      SQFitter(std::shared_ptr<Vec> camCenter=std::shared_ptr<Vec>());

      /// fits superquadrics into given point cloud, considering only given number of points
      bool fit(icl::geom::PointCloudObject& pcObj,
               const std::string& sShapePreference="bsc",
               size_t maxNumPoints=0);

      /// returns the last calculated parameters
      const SQParams& getParams() const {return params;}

      /// changes the camera center
      void setCameraCenter(const Vec &c);

    private:
      /// internal function, that fits a superquadric into given point cloud
      void fitSQ(LM::Matrix &Mx, math::Vec3 &viewDir, const std::string& sShapePreference);
      /// internal function, that prepares some data for fitting of superquadrics
      void preProcess(LM::Matrix &Mx, math::Vec3 &viewDir,
                      const std::string& sShapePreference,
                      icl::math::FixedMatrix<float,3,3> &R,
                      math::Vec3 &center, math::Vec3 &size, math::Vec3 &origin,
                      float &scale);
      /// internal function, that determines the shape parameters
      LM::Result fitShape(int i, icl::math::FixedMatrix<float,3,3> &R,
                          LM::Matrix &Mx, SQParams &params,
                          math::Vec3 &size, math::Vec3 &euler);

    private:
      SQFitter::shapeError sError;         //!< structure containing an error function
      SQFitter::eulerPosError ePError;     //!< structure containing an error function
      LM::FunctionMat sErrorFunc;          //!< error function for the shape
      LM::FunctionMat ePErrorFunc;         //!< error function for the euler angles and the position
      LM shapeLM;                          //!< Levenberg-Marquardt for the shape
      LM eulerPosLM;                       //!< Levenberg-Marquardt for the euler angles and the position
      SQParams params;                     //!< last solution
      std::shared_ptr<Vec> camCenter; //!< camera center
    };
    using SQFitterPtr = std::shared_ptr<SQFitter>;

  } // namespace geom
}
