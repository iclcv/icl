// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Configurable.h>
#include <ICLUtils/Exception.h>
#include <ICLGeom/GeomDefs.h>
#include <ICLGeom/Camera.h>

namespace icl{
  namespace geom{

    /// RANSAC-based pose estimation
    class ICLGeom_API RansacBasedPoseEstimator : public utils::Configurable{
      struct Data;  //!< internal data handling
      Data *m_data; //!< internal data pointer

      public:

      struct ICLGeom_API Result{
        Mat T;
        bool found;
        float error;
      };


      RansacBasedPoseEstimator(const geom::Camera &camera,
                                int iterations=200,
                                int minPoints=4,
                                float maxErr=5,
                               float minPointsForGoodModel=12,
                               bool storeLastConsensusSet=false);

      ~RansacBasedPoseEstimator();

      void setIterations(int iterations);

      void setMinPoints(int minPoints);

      void setMaxError(float maxError);

      void setMinPointsForGoodModel(float f);

      void setStoreLastConsensusSet(bool);

      std::vector<utils::Point32f> getLastConsensusSet();

      /// fit from planar target
      Result fit(const std::vector<utils::Point32f> &modelPoints,
                 const std::vector<utils::Point32f> &imagePoints);

      /// fit from non-planar target
      Result fit(const std::vector<geom::Vec> &modelPoints,
                 const std::vector<utils::Point32f> &imagePoints);


      /// internal utility method
      std::vector<float> fit_coplanar(const std::vector<std::vector<float> > &pts);

      /// internal utility method
      icl64f err_coplanar(const std::vector<float> &m, const std::vector<float> &p);


    };
  }
}
