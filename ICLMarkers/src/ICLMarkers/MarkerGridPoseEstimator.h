/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/MarkerGridPoseEstimator.h    **
** Module : ICLMarkers                                             **
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
    
