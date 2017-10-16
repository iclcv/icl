/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/CurvatureFeatureExtractor.h        **
** Module : ICLGeom                                                **
** Authors: Andre Ueckermann                                       **
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

#include <ICLMath/DynMatrix.h>
#include <ICLGeom/SurfaceFeatureExtractor.h>

namespace icl{
  namespace geom{

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
        static math::DynMatrix<bool> apply(const core::Img32f &depthImg, core::DataSegment<float,4> &xyz, math::DynMatrix<bool> &initialMatrix,
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

  } // namespace geom
}
