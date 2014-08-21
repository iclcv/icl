/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/SQFitter.h                         **
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
      typedef icl::math::LevenbergMarquardtFitter<float> LM; //!< Levenberg-Marquardt

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
      SQFitter(icl::utils::SmartPtr<Vec> camCenter=utils::SmartPtr<Vec>());

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
      icl::utils::SmartPtr<Vec> camCenter; //!< camera center
    };
    typedef icl::utils::SmartPtr<SQFitter> SQFitterPtr;

  } // namespace geom
}
