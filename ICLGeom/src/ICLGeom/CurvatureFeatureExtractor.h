// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Andre Ueckermann, Christof Elbrechter

#pragma once

#include <ICLMath/DynMatrix.h>
#include <ICLGeom/SurfaceFeatureExtractor.h>

namespace icl::geom {
    /// class for curvature feature (open and occluded objects).
    /** This class implements the curvature feature for feature-graph based segmentation.*/

    class CurvatureFeatureExtractor{

  	  public:

        /// Calculates the curvature feature matrix.
      /** @param depthImg the input depth image
          @param xyz the input xyz pointcloud data segment
          @param initialMatrix the initial boolean test matrix (0 test if both curved, 1 dont test, preferably an adjacency matrix)
          @param features the surface feature for the surfaces
          @param surfaces the vector of surface id vectors
          @param normals the input normals
          @param useOpenObjects true for computation of open objects
          @param useOccludedObjects true for computation of occluded objects
          @param histogramSimilarity the minimum histogram similarity
          @param distance the maximum distance between two curved objects in pixel for open objects (e.g. a cup)
          @param maxError maximum RANSAC error for object alignment detection
          @param ransacPasses number of RANSAC passes for object alignment detection
          @param distanceTolerance distance tolerance for occlusion check
          @param outlierTolerance outlier tolerance for occlusion check
          @return the boolean curvature matrix */
        static math::DynMatrixBase<bool> apply(const core::Img32f &depthImg, core::DataSegment<float,4> &xyz, math::DynMatrixBase<bool> &initialMatrix,
                          std::vector<SurfaceFeatureExtractor::SurfaceFeature> features,
                          std::vector<std::vector<int> > &surfaces, core::DataSegment<float,4> &normals, bool useOpenObjects=true, bool useOccludedObjects=true,
                          float histogramSimilarity=0.5, int distance=10, float maxError=10., int ransacPasses=20, float distanceTolerance=3., float outlierTolerance=5.);

      private:

        static bool computeOpenObject(core::DataSegment<float, 4> &normals, SurfaceFeatureExtractor::SurfaceFeature feature1, SurfaceFeatureExtractor::SurfaceFeature feature2,
                                    std::vector<int> &surface1, std::vector<int> &surface2, int distance, int w);

        static bool computeOccludedObject(const core::Img32f &depthImg, core::DataSegment<float,4> &xyz, core::DataSegment<float, 4> &normals,
                                    SurfaceFeatureExtractor::SurfaceFeature feature1, SurfaceFeatureExtractor::SurfaceFeature feature2,
                                    std::vector<int> &surface1, std::vector<int> &surface2, int w, float maxError, int ransacPasses, float distanceTolerance, float outlierTolerance);

        static float computeConvexity(core::DataSegment<float, 4> &normals, SurfaceFeatureExtractor::SurfaceFeature feature, std::vector<int> &surface, int w);

        static std::pair<utils::Point,utils::Point> computeExtremalBins(SurfaceFeatureExtractor::SurfaceFeature feature);

        static std::pair<utils::Point,utils::Point> backproject(core::DataSegment<float, 4> &normals,
                        std::pair<utils::Point,utils::Point> &histo1ExtremalBins, std::vector<int> &surface1, int w);

        static std::vector<int> backprojectPointIDs(core::DataSegment<float,4> &normals, utils::Point bin, std::vector<int> &surface);

        static utils::Point computeMean(std::vector<int> &imgIDs, int w);

        static std::vector<utils::Point> createPointsFromIDs(std::vector<int> &imgIDs, int w);

        static std::vector<Vec> createPointsFromIDs(core::DataSegment<float,4> &xyz, std::vector<int> &imgIDs);

        static float computeConvexity(std::pair<utils::Point,utils::Point> histoExtremalBins, std::pair<utils::Point,utils::Point> imgBackproject);

        static float linePointDistance(std::pair<Vec,Vec> line, Vec point);

        static utils::Point idToPoint(int id, int w);

    };

  } // namespace icl::geom