// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Andre Ueckermann, Christof Elbrechter

#pragma once

#include <ICLMath/DynMatrix.h>

namespace icl::geom {
    /**
     This class is used in the ObjectEdgeDetector implementations */
    class ICLGeom_API ObjectEdgeDetectorData{


     public:
      struct m_params{
        int medianFilterSize;
        int normalRange;
        int normalAveragingRange;
        int neighborhoodMode;
        int neighborhoodRange;
        float binarizationThreshold;
        bool useNormalAveraging;
        bool useGaussSmoothing;
      };

      struct m_kernel{
        float norm;
	    math::DynMatrix<float> kernel;
	    int l;
	    int kSize;
	    int rowSize;
      };


      /// Create new ObjectEdgeDetectorData
      ObjectEdgeDetectorData();

      ///Destructor
      virtual ~ObjectEdgeDetectorData();

      /// Returns Kernel of a given size
      /** @param size the kernel size
          @return the kernel */
      m_kernel getKernel(int size);

      /// Returns the initial parameters.
      /** @return the inital parameters */
      m_params getParameters();
    };
  }