// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Andre Ueckermann, Christof Elbrechter

#pragma once

#include <ICLGeom/GeomDefs.h>
#include <ICLCore/DataSegment.h>
#include <ICLCore/Channel.h>

namespace icl{
  namespace geom{

    /// Support class for segmentation algorithms.
    /** This class provides supporting methods for segmentation algorithms.*/

    class SegmenterUtils{

  	  public:
  	    enum Mode {BEST, GPU, CPU};

  	    /// Constructor
        /** Constructs an object of this class.
            @param mode the selected mode: CPU, GPU or BEST (uses GPU if available)*/
        SegmenterUtils(Mode mode=BEST);


        /// Destructor
        ~SegmenterUtils();


        /// Creates a color image (e.g. for pointcloud coloring) from a given segmentation label image.
        /** @param labelImage the input label image
            @return the output color image.
        */
        core::Img8u createColorImage(core::Img32s &labelImage);

        /// Creates the mask image for segmentation (including 3D ROI).
        /** @param xyzh the input pointcloud (point position)
            @param depthImage the input depth image
            @xMin parameter for ROI (in mm for world coordinates)
            @xMax parameter for ROI (in mm for world coordinates)
            @yMin parameter for ROI (in mm for world coordinates)
            @yMax parameter for ROI (in mm for world coordinates)
            @zMin parameter for ROI (in mm for world coordinates)
            @zMax parameter for ROI (in mm for world coordinates)
            @return the output mask image.
        */
        core::Img8u createROIMask(core::DataSegment<float,4> &xyzh, core::Img32f &depthImage,
                    float xMin, float xMax, float yMin, float yMax, float zMin=-10000, float zMax=10000);

        /// Creates the mask image for segmentation.
        /** @param depthImage the input depth image
            @return the output mask image.
        */
        core::Img8u createMask(core::Img32f &depthImage);

        /// Minimizes the label ID changes from frame to frame. The overlaps between the current and the previous label image are calculated and relabeled for the result.
        /** @param labelImage the input label image
            @return the stabelized output label image.
        */
        core::Img32s stabelizeSegmentation(core::Img32s &labelImage);

        /// Calculates the adjacency between segments. Use edgePointAssignmentAndAdjacencyMatrix(...) if edge point assignment is needed as well.
        /** @param xyzh the input pointcloud (point position)
            @param labelImage the input label image
            @param maskImage the input mask image
            @param radius in pixel (distance of surfaces/segments around separating edge)
            @param euclideanDistance the maximum euclidean distance between adjacent surfaces/segments
            @param numSurfaces the number of surfaces/segments in the label image
            @return the adjacency matrix.
        */
        math::DynMatrixBase<bool> calculateAdjacencyMatrix(core::DataSegment<float,4> &xyzh, core::Img32s &labelImage,
                              core::Img8u &maskImage, int radius, float euclideanDistance, int numSurfaces);

        /// Assigns the edge points to the surfaces. Use edgePointAssignmentAndAdjacencyMatrix(...) if adjacency matrix is needed as well.
        /** @param xyzh the input pointcloud (point position)
            @param labelImage the input label image (changed by the method)
            @param maskImage the input mask image (changed by the method)
            @param radius in pixel (distance of surfaces/segments around separating edge)
            @param euclideanDistance the maximum euclidean distance between adjacent surfaces/segments
            @param numSurfaces the number of surfaces/segments in the label image
        */
        void edgePointAssignment(core::DataSegment<float,4> &xyzh, core::Img32s &labelImage,
                              core::Img8u &maskImage, int radius, float euclideanDistance, int numSurfaces);

        /// Calculates the adjacency between segments and assigns the edge points to the surfaces.
        /** @param xyzh the input pointcloud (point position)
            @param labelImage the input label image (changed by the method)
            @param maskImage the input mask image (changed by the method)
            @param radius in pixel (distance of surfaces/segments around separating edge)
            @param euclideanDistance the maximum euclidean distance between adjacent surfaces/segments
            @param numSurfaces the number of surfaces/segments in the label image
            @return the adjacency matrix.
        */
        math::DynMatrixBase<bool> edgePointAssignmentAndAdjacencyMatrix(core::DataSegment<float,4> &xyzh, core::Img32s &labelImage,
                              core::Img8u &maskImage, int radius, float euclideanDistance, int numSurfaces);

        /// Extracts the segments from a label image.
        /** @param labelImage the input label image
            @return a vector of pointID vectors.
        */
        std::vector<std::vector<int> > extractSegments(core::Img32s &labelImage);

        /// Relabels the label image.
        /** @param labelImage the input/output label image
            @param assignment a vector of vectors with label ids. Each label from the inner id is replaced by the outer vector id.
            @param maxOldLabel the maximum id of the old labels (optional)
        */
        void relabel(core::Img32s &labelImage, std::vector<std::vector<int> > &assignment, int maxOldLabel=0);

        /// Checks if there is occlusion between two points (depth of all points on or in front of an augmented line).
        /** @param depthImage the input depth image
            @param p1 the first image point
            @param p2 the second image point
            @param distanceTolerance the distance tolerance in depthUnits
            @param outlierTolerance maximum number of outlier points in percent
            @return true if occluded.
        */
        static bool occlusionCheck(core::Img32f &depthImage, utils::Point p1, utils::Point p2, float distanceTolerance=3., float outlierTolerance=5.);

        /// Creates the label vectors from a given label image
        /** @param labelImage the input label image
            @return the vector of id vectors
        */
        static std::vector<std::vector<int> > createLabelVectors(core::Img32s &labelImage);

      private:

        struct Data;  //!< internal data type
        Data *m_data; //!< internal data pointer

        void createColorImageCL(core::Img32s &labelImage, core::Img8u &colorImage);

        void createColorImageCPU(core::Img32s &labelImage, core::Img8u &colorImage);

        std::vector<int> calculateLabelReassignment(int countCur, int countLast, core::Channel32s &labelImageC, core::Channel32s &lastLabelImageC, utils::Size size);

        math::DynMatrixBase<bool> edgePointAssignmentAndAdjacencyMatrixCL(core::DataSegment<float,4> &xyzh, core::Img32s &labelImage,
                              core::Img8u &maskImage, int radius, float euclideanDistance, int numSurfaces, bool pointAssignment);

        math::DynMatrixBase<bool> edgePointAssignmentAndAdjacencyMatrixCPU(core::DataSegment<float,4> &xyzh, core::Img32s &labelImage,
                              core::Img8u &maskImage, int radius, float euclideanDistance, int numSurfaces, bool pointAssignment);

        static float dist3(const Vec &a, const Vec &b){
          return norm3(a-b);
        }

    };

  } // namespace geom
}
