/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/                             **
**          MarkerGridBasedUndistortionOptimizer.h                 **
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

namespace icl{
  namespace markers{
    class MarkerGridBasedUndistortionOptimizer{
      typedef AdvancedMarkerGridDetector::Marker Marker;
      typedef AdvancedMarkerGridDetector::MarkerGrid MarkerGrid;
      struct Data;
      Data *m_data;


      void undistort(const MarkerGrid &src,
                     MarkerGrid &dst, const float k[9]) const;

      public:
      MarkerGridBasedUndistortionOptimizer();
      ~MarkerGridBasedUndistortionOptimizer();
      
      int size() const;
      void add(const MarkerGrid &grid);
      
      void clear();

      void setUseOpenCL(bool on);
      
      /// k = k0,k1,k2,k3,k4, w/2 + ix-offset, h/2 + iy-offset
      float computeError(const float k[9]);
      
      std::vector<float> optimizeSample(const float kInit[9],
                                        int idx, float min, float max, 
                                        const std::vector<int> steps=std::vector<int>(3,10));

      std::vector<float> optimizeAutoSample(const utils::Size &imageSize);

      std::vector<float> optimizeAutoSimplex(const utils::Size &imageSize);
      
      
    };
  }
}
