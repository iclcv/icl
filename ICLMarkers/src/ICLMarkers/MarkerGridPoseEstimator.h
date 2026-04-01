// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLMarkers/AdvancedMarkerGridDetector.h>
#include <ICLUtils/Configurable.h>

#include <ICLGeom/Camera.h>

namespace icl{
  namespace markers{

    /// Utility class that can estimate the pose of a defined marker grid
    /** The internal geom.:CoplanarPointPoseEstimator is added as child-configurable */
	  class ICLMarkers_API MarkerGridPoseEstimator : public utils::Configurable{
      struct Data;   //!< internal data structure
      Data *m_data;  //!< internal data pointer

      public:
      /// default constructor
      MarkerGridPoseEstimator();

      /// destructor
      ~MarkerGridPoseEstimator();

      /// Computes the pose of the given deteced marker-grid
      /** All pose detection options are accessible to the Configurable interface */
      geom::Mat computePose(const AdvancedMarkerGridDetector::MarkerGrid &grid,
                            const geom::Camera &cam);
    };

  }
}
