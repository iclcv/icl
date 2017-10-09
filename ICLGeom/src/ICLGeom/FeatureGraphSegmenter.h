/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/FeatureGraphSegmenter.h            **
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

#include <ICLCore/DataSegment.h>
#include <ICLGeom/SurfaceFeatureExtractor.h>
#include <ICLGeom/PointCloudSegment.h>

namespace icl{
  namespace geom{
    /**
       This class implements feature graph segmentation for depth images. It uses OpenCL for hardware parallelization if a compatible GPU is found. */
    class ICLGeom_API FeatureGraphSegmenter{

     public:

      enum Mode {BEST, GPU, CPU};

      /// Constructor
      /** Constructs an object of this class. All default parameters are set. Use setters for desired values.
          @param mode GPU, CPU and BEST (default) */
      FeatureGraphSegmenter(Mode mode=BEST);

      /// Destructor
      ~FeatureGraphSegmenter();

      /// One line call for the surface feature segmentation.
      /** @param xyz the xyzh DataSegment from the PointCloudObject class
          @param edgeImg the edge image from the ObjectEdgeDetector class
          @param depthImg the input depth image
          @param normals the input normals
          @param stabelize frame-to-frame cross-correlation to achieve stable object labels for each frame
          @param useROI true for 3D region of interest (set by setROI() )
          @param useCutreeAdjacency use the cutfree adjacency feature
          @param useCoplanarity use the coplanarity feature
          @param useCurvature use the curvature feature
          @param useRemainingPoints use the remaining points assignment
          @return the color image of the segments */
      core::Img8u apply(core::DataSegment<float,4> xyz, const core::Img8u &edgeImg, const core::Img32f &depthImg,
                  core::DataSegment<float,4> normals=NULL, bool stabelize=true, bool useROI=false,
                  bool useCutfreeAdjacency=true, bool useCoplanarity=true, bool useCurvature=true, bool useRemainingPoints=true);

  	  /// One line call for the hierarchical surface feature segmentation.
      /** @param xyz the xyzh DataSegment from the PointCloudObject class
          @param rgb the rgba32f DataSegment from the PointCloudObject class
          @param edgeImg the edge image from the ObjectEdgeDetector class
          @param depthImg the input depth image
          @param normals the input normals
          @param useROI true for 3D region of interest (set by setROI() )
          @param useCutreeAdjacency use the cutfree adjacency feature
          @param useCoplanarity use the coplanarity feature
          @param useCurvature use the curvature feature
          @param useRemainingPoints use the remaining points assignment
          @param weightCutreeAdjacency use the cutfree adjacency feature
          @param weightCoplanarity use the coplanarity feature
          @param weightCurvature use the curvature feature
          @param weightRemainingPoints use the remaining points assignment
          @return the hierachical segmentation tree */
  	  std::vector<PointCloudSegmentPtr> applyHierarchical(core::DataSegment<float,4> xyz, core::DataSegment<float,4> rgb, const core::Img8u &edgeImg, const core::Img32f &depthImg,
                  core::DataSegment<float,4> normals=NULL, bool useROI=false,
                  bool useCutfreeAdjacency=true, bool useCoplanarity=true, bool useCurvature=true, bool useRemainingPoints=true,
                  float weightCutfreeAdjacency=1.0, float weightCoplanarity=0.9, float weightCurvature=0.8, float weightRemainingPoints=0.7);

      /// Sets the ROI in world coordinates
      /**       @param xMin xMin in mm
                @param xMax xMax in mm
                @param yMin yMin in mm
                @param yMax yMax in mm
                @param zMin zMin in mm
                @param zMax zMax in mm*/
      void setROI(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax);

      /// Sets the minimum segment size (default 25)
      /**       @param size minimum segment size in points */
      void setMinSurfaceSize(unsigned int size);

      /// Sets the edge point assignment parameters
      /**       @param distance the maximum euclidean distance between two points
                @param radius the maximum radius in pixel */
      void setAssignmentParams(float distance, int radius);

      /// Sets the cutfree adjacency feature parameters
      /**       @param euclideanDistance the euclidean distance for RANSAC test
                @param passes the number of RANSAC passes
                @param tolerance the outlier tolerance in points for RANSAC
                @param minAngle the minimum angle between two adjacent surfaces */
      void setCutfreeParams(float euclideanDistance, int passes, int tolerance, float minAngle);

      /// Sets the coplanarity feature parameters
      /**       @param maxAngle the maximum angle between two coplanar surfaces
                @param distanceTolerance distance tolerance for occlusion check
                @param outlierTolerance outlier tolerance for occlusion check
                @param numTriangles number of triangles for common plane test
                @param numScanlines the number of scanlines for occlusion check */
      void setCoplanarityParams(float maxAngle, float distanceTolerance, float outlierTolerance, int numTriangles, int numScanlines);

      /// Sets the curvature feature parameters
      /**       @param histogramSimilarity the minimum similarity score for matching
                @param useOpenObjects test for open curved objects (e.g. a cup)
                @param maxDistance the maximum distance in pixel between the two surfaces
                @param useOccludedObjects test for occluded curved objects
                @param maxError the maximum error in mm for alignment test
                @param ransacPasses the number of RANSAC passes for alignment test
                @param distanceTolerance distance tolerance for occlusion check
                @param outlierTolerance outlier tolerance for occlusion check */
      void setCurvatureParams(float histogramSimilarity, bool useOpenObjects, int maxDistance, bool useOccludedObjects, float maxError,
                              int ransacPasses, float distanceTolerance, float outlierTolerance);

            /// Sets the remaining points assignment parameters
      /**       @param minSize the minimum size of a remaining point blob
                @param euclideanDistance the maximum euclidean distance for point clustering
                @param radius the radius for local minima search
                @param assignEuclideanDistance the euclidean distance for assigning to neighbouring surfaces
                @param supportTolerance the max tolerance for detecting a blob as supported and therefore separate object */
      void setRemainingPointsParams(int minSize, float euclideanDistance, int radius, float assignEuclideanDistance, int supportTolerance);

      /// Sets the graphcut threshold
      /**       @param threshold the maximum weight of a cut */
      void setGraphCutThreshold(float threshold);

	  std::vector<SurfaceFeatureExtractor::SurfaceFeature> getSurfaceFeatures();

      /// Returns the segment blobs.
      /**        @return a vector of segments. Every entry contains a vector with the surface indices */
      std::vector<std::vector<int> > getSegments();

      /// Returns the surface cluster from the pre-segmentation.
      /**        @return a vector of surfaces. Every entry contains a vector with the point indices */
      std::vector<std::vector<int> > getSurfaces();

      /// Returns the labeled segment image.
      /**        @param stabelize true for consistent label IDs.
                 @return the labeled segment image */
      core::Img32s getLabelImage(bool stabelize);

      /// Returns the colored segment image. Also returned by the apply method.
      /**        @param stabelize true for consistent label colors.
                 @return the colored segment image */
      core::Img8u getColoredLabelImage(bool stabelize);

      /// Returns the mask image.
      /**        @return the mask image of the segmentation */
      core::Img8u getMaskImage();

     private:

      void surfaceSegmentation(core::DataSegment<float,4> &xyz, const core::Img8u &edgeImg, const core::Img32f &depthImg, int minSurfaceSize=25, bool useROI=false);

      std::vector<PointCloudSegmentPtr> createHierarchy(math::DynMatrix<float> &probabilityMatrix, core::DataSegment<float,4> &xyz, core::DataSegment<float,4> &rgb);

      struct Data;  //!< internal data type
      Data *m_data; //!< internal data pointer

    };
  } // namespace geom
}
