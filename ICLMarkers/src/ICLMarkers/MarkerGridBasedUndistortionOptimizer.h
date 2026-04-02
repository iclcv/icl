// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLMarkers/AdvancedMarkerGridDetector.h>

namespace icl::markers {
	  class ICLMarkers_API MarkerGridBasedUndistortionOptimizer{
      using Marker = AdvancedMarkerGridDetector::Marker;
      using MarkerGrid = AdvancedMarkerGridDetector::MarkerGrid;
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