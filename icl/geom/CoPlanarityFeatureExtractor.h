// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Andre Ueckermann, Christof Elbrechter

#pragma once

#include <icl/math/DynMatrix.h>
#include <icl/geom/SurfaceFeatureExtractor.h>

namespace icl::geom {
  /// class for coplanarity feature.
  /** This class implements the coplanarity feature for feature-graph based segmentation.*/

  class CoPlanarityFeatureExtractor{

	  public:

      /// Calculates the coplanarity feature matrix.
    /** @param initialMatrix the initial boolean test matrix (0 test if both planar, 1 dont test, preferably an adjacency matrix)
        @param features the surface feature for the surfaces
        @param depthImage the input depthImage
        @param surfaces the vector of surface id vectors
        @param maxAngle the maximum angle for surface orientation similarity test
        @param distanceTolerance the maximum distance in mm for occlusion check
        @param outlierTolerance the maximum number of outlier points in percent for occlusion check
        @param triangles number of combined surface tests
        @param scanlines number of occlusion tests
        @return the boolean coplanarity matrix */
      static math::DynMatrixBase<bool> apply(math::DynMatrixBase<bool> &initialMatrix, std::vector<SurfaceFeatureExtractor::SurfaceFeature> features,
                        const core::Img32f &depthImage, std::vector<std::vector<int> > &surfaces, float maxAngle=30,
                        float distanceTolerance=3, float outlierTolerance=5, int triangles=50, int scanlines=9);

    private:

      static float getAngle(Vec n1, Vec n2);
      static utils::Point getRandomPoint(std::vector<int> surface, int imgWidth);
      static Vec getNormal(Vec p1, Vec p2, Vec p3);
      static bool criterion1(Vec n1, Vec n2, float maxAngle);
      static bool criterion2(const core::Img32f &depthImage, std::vector<int> &surface1, std::vector<int> &surface2,
                            Vec n1, Vec n2, float maxAngle, int triangles);
      static bool criterion3(const core::Img32f &depthImage, std::vector<int> &surface1, std::vector<int> &surface2,
                            float distanceTolerance, float outlierTolerance, int scanlines);
  };

  } // namespace icl::geom