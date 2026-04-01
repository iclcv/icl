// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Andre Ueckermann, Christof Elbrechter

#pragma once

#include <ICLCore/DataSegment.h>
#include <ICLCore/Img.h>
#include <ICLGeom/SurfaceFeatureExtractor.h>

namespace icl{
  namespace geom{
    /**
       This class implements the cutfree adjacency feature for feature-graph based segmentation.*/
    class CutfreeAdjacencyFeatureExtractor{

     public:

      enum Mode {BEST, GPU, CPU};

      /// Constructor
      /** Constructs an object of this class.
          @param mode GPU, CPU and BEST (default) */
      CutfreeAdjacencyFeatureExtractor(Mode mode=BEST);

      /// Destructor
      ~CutfreeAdjacencyFeatureExtractor();

      /// Calculates the cutfree adjacency feature matrix.
      /** @param xyzh the xyzh DataSegment from the PointCloudObject class
          @param surfaces the vector of surface id vectors
          @param testMatrix the initial boolean test matrix (1 test, 0 dont test, preferably an adjacency matrix)
          @param euclideanDistance the maximum euclidean distance for RANSAC in mm
          @param passes the RANSAC passes
          @param tolerance the RANSAC tolerance in number of points (outlier)
          @param labelImage the label image
          @return the boolean cutfree adjacency matrix */
       math::DynMatrix<bool> apply(core::DataSegment<float,4> &xyzh,
                std::vector<std::vector<int> > &surfaces, math::DynMatrix<bool> &testMatrix, float euclideanDistance,
                int passes, int tolerance, core::Img32s labelImage);

       /// Calculates the cutfree adjacency feature matrix with minimum angle constraint.
      /** @param xyzh the xyzh DataSegment from the PointCloudObject class
          @param surfaces the vector of surface id vectors
          @param testMatrix the initial boolean test matrix (1 test, 0 dont test, preferably an adjacency matrix)
          @param euclideanDistance the maximum euclidean distance for RANSAC in mm
          @param passes the RANSAC passes
          @param tolerance the RANSAC tolerance in number of points (outlier)
          @param labelImage the label image
          @param feature the surface feature for the surfaces
          @param minAngle the minimum angle for combination
          @return the boolean cutfree adjacency matrix */
       math::DynMatrix<bool> apply(core::DataSegment<float,4> &xyzh,
                std::vector<std::vector<int> > &surfaces, math::DynMatrix<bool> &testMatrix, float euclideanDistance,
                int passes, int tolerance, core::Img32s labelImage,
                std::vector<SurfaceFeatureExtractor::SurfaceFeature> feature, float minAngle);

     private:

      struct Data;  //!< internal data type
      Data *m_data; //!< internal data pointer

    };
  } // namespace geom
}
