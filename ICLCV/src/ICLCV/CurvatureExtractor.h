// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Tobias Röhlig, Christof Elbrechter

#pragma once

#include <ICLCore/Core.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLMath/DynMatrix.h>
#include <ICLCV/ImageRegion.h>
#include <stdint.h>

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
        using DMatF = math::DynMatrix<float>;
        /// Histogram type
        using FloatHist = std::vector<float>;
        /// Array of points
        using PointVector = std::vector<utils::Point>;
        /// Internal used index vector
        using IndexVector = std::vector<int>;

        /**
         * @brief RegionCurvature Constructor
         * @param curv_radius radius to compute the curvature for each point
         * @param steps used only if the inidecs in extractContourCurvature() are not given. In this case, only each steps' point curvature is computed
         * @param thinned_contour use thinned contour when extractRegionCurvature() is used
         */
        CurvatureExtractor(const uint32_t curv_radius, const uint32_t steps = 1,
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
                                     const uint32_t regionLookupId,
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
                                    const std::vector<uint32_t> &regionLookupIds,
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
        uint32_t getCurvatureRadius() { return m_curv_radius; }
        /**
         * @brief setCurvatureRadius
         * @param radius the discrete radius that is used to compute the curvature
         */
        void setCurvatureRadius(const uint32_t radius) { m_curv_radius = radius; }

        /**
         * @brief getStepSize
         * @return the steps between the points the curvature is computed for
         */
        uint32_t getStepSize() { return m_steps; }
        /**
         * @brief setStepSize
         * @param steps the steps between the points the curvature is computed for
         */
        void setStepSize(const uint32_t steps) { m_steps = steps; }

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
        uint32_t m_curv_radius;
        /**
         * @brief m_steps
         * Steps between points the curvature is computed for. Used for auto-fill indices in extractContourCurvature().
         */
        uint32_t m_steps;

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
