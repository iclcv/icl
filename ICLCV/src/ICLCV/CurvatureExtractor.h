/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/CurvatureExtractor.h                   **
** Module : ICLCV                                                  **
** Authors: Tobias Röhlig                                          **
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

#include <ICLCore/Core.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLMath/DynMatrix.h>
#include <ICLCV/ImageRegion.h>

namespace icl {
    namespace cv {

    /**
     * Curvature evaluation from "Contact-free and pose-invariant hand-biometric-based personal identification system using RGB and depth data"
     * by Can Wang, Hong Liu, Xing Liu
     * Journal of Zhejiang University-SCIENCE C (Computers & Electronics) 2014
     * @brief The RegionCurvature class
     */
    class ICLCV_API CurvatureExtractor : utils::Uncopyable {

    public:

        /// Internal used matrix type
        typedef math::DynMatrix<float> DMatF;
        /// Histogram type
        typedef std::vector<float> FloatHist;
        /// Array of points
        typedef std::vector<utils::Point> PointVector;
        /// Internal used index vector
        typedef std::vector<int> IndexVector;

        /**
         * @brief RegionCurvature Constructor
         * @param curv_radius radius to compute the curvature for each point
         * @param steps used only if the inidecs in extractContourCurvature() are not given. In this case, only each steps' point curvature is computed
         * @param thinned_contour use thinned contour when extractRegionCurvature() is used
         */
        CurvatureExtractor(const uint curv_radius, const uint steps = 1,
                           const bool thinned_contour = true);

        /**
         * @brief extractContourCurvature
         *
         * Extracts the curvature histogram out of a given contour.
         *
         * @param contour a vector of points in the contour
         * @param insideLookup lookup image for the region-contour to decide between a convex or concave curve. The corresponding region in this image should be filled with regionLookupId
         * @param regionLookupId id used for this region-contour in insideLookup
         * @param indices indices of the points in contour the curvature should be computed for. Will be auto-filled by this function using m_steps if it is empty.
         * @param hist historgram of the curvature for each point in indices
         * @param dist distance matrix for points in indices
         * @param curvature curvature matrix for points in indices
         */
        void extractContourCurvature(const PointVector &contour,
                                     const core::Img8u &insideLookup,
                                     const uint regionLookupId,
                                     std::vector<int> &indices,
                                     FloatHist &hist,
                                     DMatF &dist, DMatF &curvature);

        /**
         * @brief extractRegionCurvature computes the curvature historgrams for each ImageRegion in regions. Uses extractContourCurvature() to compute each curvature.
         * @param regions vector of ImageRegions
         * @param insideLookup lookup image for the image regions to decide between a convex or concave curve. The corresponding regions in this image should be filled with regionLookupIds
         * @param regionLookupIds region ids used in insideLookup. Must be of the same size as regions
         * @param curvatureHists will be filled by this function and should contain the curvature histograms for each region in regions.
         */
        void extractRegionCurvature(const std::vector<ImageRegion> &regions,
                                    const core::Img8u &insideLookup,
                                    const std::vector<uint> &regionLookupIds,
                                    std::vector<FloatHist> &curvatureHists);

        /**
         * @brief getDistanceMatrices
         * @return the distance matrices for each region
         */
        const std::vector< DMatF > &getDistanceMatrices() { return m_distances; }

        /**
         * @brief getCurvatureMatrices
         * @return the curvature matrices for each region
         */
        const std::vector< DMatF > &getCurvatureMatrices() { return m_curvatures; }

        /**
         * @brief getUsedIndicesMatrices
         * @return the indices of the points the curvature is computed for
         */
        const std::vector< IndexVector > &getUsedIndices() { return m_used_indices; }

        /**
         * @brief getCurvatureRadius
         * @return the discrete radius that is used to compute the curvature
         */
        uint getCurvatureRadius() { return m_curv_radius; }
        /**
         * @brief setCurvatureRadius
         * @param radius the discrete radius that is used to compute the curvature
         */
        void setCurvatureRadius(const uint radius) { m_curv_radius = radius; }

        /**
         * @brief getStepSize
         * @return the steps between the points the curvature is computed for
         */
        uint getStepSize() { return m_steps; }
        /**
         * @brief setStepSize
         * @param steps the steps between the points the curvature is computed for
         */
        void setStepSize(const uint steps) { m_steps = steps; }

        /**
         * @brief getUseThinnedContour
         * @return true if thinned contour is used to extract boundary from ImageRegion
         */
        bool getUseThinnedContour() { return m_thinned_contour; }
        /**
         * @brief setUseThinnedContour
         * @param thinned_contour true if thinned contour is used to extract boundary from ImageRegion
         */
        void setUseThinnedContour(const bool thinned_contour) { m_thinned_contour = thinned_contour; }

    protected:

        /**
         * @brief m_curv_radius
         * Curvature of a point p_i is the sum of relations between the direct
         * distance and geodesic distance of p_i+k to point p_i-k where k < m_curv_radius.
         */
        uint m_curv_radius;
        /**
         * @brief m_steps
         * Steps between points the curvature is computed for. Used for auto-fill indices in extractContourCurvature().
         */
        uint m_steps;

        /// Whether use thinned contour of ImageRegions or not.
        bool m_thinned_contour;

        /// Distance matrices for each region
        std::vector< DMatF > m_distances;
        /// Curvature matrices for each region
        std::vector< DMatF > m_curvatures;
        /// Indices used for each region. See m_steps.
        std::vector< IndexVector > m_used_indices;

        /**
         * @brief restOp for circle-like rest operations (int only):
         * @param x
         * @param y
         * @return r=x%y; return r<0 ? y+r : r;
         */
        inline
        int restOp(int x, int y) {
            int r = x%y;
            return r<0 ? y+r : r;
        }

    };

    } //cv
} //icl
