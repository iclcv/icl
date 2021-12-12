/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/RansacBasedPoseEstimator.h         **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter                                    **
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
