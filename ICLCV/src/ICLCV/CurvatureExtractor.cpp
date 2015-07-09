/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/CurvatureExtractor.cpp                 **
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


#include <ICLCV/CurvatureExtractor.h>

#include <ICLUtils/StackTimer.h>

using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;

namespace icl {
    namespace cv {

    CurvatureExtractor::CurvatureExtractor(const uint32_t curv_radius,
                                     const uint32_t steps,
                                     const bool thinned_contour)
        : m_curv_radius(curv_radius), m_steps(steps),
          m_thinned_contour(thinned_contour) {

    }

    void CurvatureExtractor::extractContourCurvature(const PointVector &contour,
                                                  const Img8u &insideLookup,
                                                  const uint32_t regionLookupId,
                                                  std::vector<int> &indices,
                                                  FloatHist &hist,
                                                  DMatF &dist,
                                                  DMatF &curvature) {

        BENCHMARK_THIS_FUNCTION;

        if (m_steps<1)
            m_steps = 1;

        //const PointVector &contour = region.getBoundary(m_thinned_contour);

        const uint32_t dim = contour.size();

        ICLASSERT_RETURN((dim > 1 || dim >= m_curv_radius*m_steps));

        // create the distance matrix:
        dist = DMatF(dim,dim,0.0f);

    //#pragma omp parallel for schedule(dynamic)
        for (uint32_t col = 0; col < dim; ++col) {
            const Point &p1 = contour[col];
            for (uint32_t row = col+1; row < dim; ++row) {
                const Point &p2 = contour[row];
                dist(col,row) = dist(row,col) = fabs(p1.distanceTo(p2));
            }
        }

        // if no initial indices for interesting regions R are given, we initialize
        // them to the whole contour according to the steps given in 'm_steps'
        if (indices.empty()) {
            indices.reserve(std::floor(dim/(float)m_steps));
            for (uint32_t i = 0; i < dim; i+=m_steps)
                indices.push_back(i);
        }

        // compute curvature
        curvature = DMatF(indices.size(),m_curv_radius);
        hist.resize(indices.size());

    //#pragma omp parallel for schedule(dynamic)
        for (uint32_t i = 0; i < indices.size(); ++i) {
            const int index = indices[i];
            float h_i = 0;
            for (uint32_t s = 1; s < m_curv_radius; ++s) {
                // the curent indices i-s_j and i+s_j
                const uint32_t min = restOp((index-s),dim);
                const uint32_t max = restOp((index+s),dim);
                const Point32f p_1 = contour[min];
                const Point32f p_2 = contour[max];

                // look whether this is a convex region or not:
                float delta = -1;
                Point32f p_m = (p_1+p_2) * 0.5f;
                if (insideLookup(round(p_m[0]),round(p_m[1]),0) == regionLookupId) {
                    delta = 1;
                }
                // the geodesic length 'l' of a curve
                float l = 0;
				for (uint32_t k = 0; k < 2 * s; ++k) {
					const uint32_t i1 = (min + k) % dim;
					const uint32_t i2 = (min + k + 1) % dim;
                    l += dist(i1,i2);
                }
                // compute the curavature 'k_i'
                float k_i = delta * (l / dist(min,max));
                if (std::isinf(k_i)) {
                    k_i = delta;
                }
                // fill the curvature matrix
                curvature(i,s) = k_i;
                // histogram value
                h_i += k_i;
            }
            // fill the histogram that can be used to find minima and maxima along the contour points
            hist[i] = h_i;
        }

    }

    void CurvatureExtractor::extractRegionCurvature(const std::vector<ImageRegion> &regions,
                                                 const Img8u &insideLookup,
                                                 const std::vector<uint32_t> &regionLookupIds,
                                                 std::vector<FloatHist> &curvatureHists) {

        BENCHMARK_THIS_FUNCTION;

        ICLASSERT_RETURN((regions.size() == regionLookupIds.size()));

		uint32_t dim = regions.size();

        // prepare internal memory
        curvatureHists.resize(dim);
        m_distances.resize(dim);
        m_curvatures.resize(dim);
        m_used_indices.resize(dim);

		for (uint32_t i = 0; i < regions.size(); ++i) {
            const ImageRegion &region = regions[i];
            extractContourCurvature(region.getBoundary(m_thinned_contour),
                                    insideLookup,regionLookupIds[i],
                                    m_used_indices[i],
                                    curvatureHists[i],
                                    m_distances[i],
                                    m_curvatures[i]);
        }
    }

    } //cv
} //icl
