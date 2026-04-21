// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Sergius Gaulik, Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Point.h>
#include <icl/utils/Point32f.h>
#include <icl/core/Img.h>
#include <icl/core/Types.h>
#include <vector>

namespace icl::cv {
  /// Ramer-Douglas-Peucker algorithm
  /** The algorithm takes an array of contour points as input. If a polygon should be
      approximated, then the two farthest points are found approximately. This points divide
      the polygon in two curves. For all curves the following is done recursively:
      First the perpendicular distance is calculated between each point and the line, which
      contains the first and last point of the curve. Afterwards the maximum distance is
      compared with an epsilon. The algorithm divides the curve further if the value is
      higher than the epsilon. Otherwise the begin and end point are taken for the result
      approximation. (see http://en.wikipedia.org/wiki/Ramer-Douglas-Peucker_algorithm for
      more information)

      Parameters for the algorithm are:
    - <b>epsilon</b>
      denotes the maximum ratio between the perpendicular and the line distance,
      that is used to filter points
    - <b>max_corners</b>
      denotes the maximum number of corners; if the number of corners is higher, the
      algorithm return an empty list
    - <b>search_iters</b>
      denotes the number of steps to find the two farthest points of a polygon
  **/
  class ICLCV_API RDPApproximation {

    struct ChainPoint : utils::Point32f {
      ChainPoint *prev;
      ChainPoint *next;
    };

  public:
    /// default constructor with given arguments
    RDPApproximation(float epsilon = 0.1f, int max_corners = 0, int search_iters = 3) :
      epsilon(epsilon), max_corners(max_corners), search_iters(search_iters) {};

    /// approximates a polygon
    /**
      * the param polygon shows, if the list is a polygon or a curve
      */
    const std::vector<utils::Point32f> &approximate(const std::vector<utils::Point> &poly, bool polygon = true);
    const std::vector<utils::Point32f> &approximate(const std::vector<utils::Point32f> &poly, bool polygon = true);
    const std::vector<utils::Point32f> &approximate(const utils::Point *begin, const utils::Point *end, const int size, bool polygon = true);

    /// draws all points to the first channel of the given image with the given value
    void drawAllPoints(core::ImgBase *img, const icl64f &value);

    /// returns the last approximation result
    inline const std::vector<utils::Point32f> &getLastApproximation() const {
      return approximation;
    }

    inline void setEpsilon(float value){ epsilon = value; }
    inline void setMaxCorners(float value){ max_corners = value; }
    inline void setSearchIterations(float value){ search_iters = value; }
    inline float getEpsilon() const { return epsilon; }
    inline float getMaxCorners() const { return max_corners; }
    inline float getSearchIterations() const { return search_iters; }

  private:
    /// parameters
    float epsilon;
    int max_corners;
    int search_iters;

    /// approximates a curve
    void approximateCurve(const ChainPoint *first, const ChainPoint *last);

    /// tries to approximates a curve with a maximum number of points
    /**
      * the param cap shows how much points can be in between of the first and the last point
      */
    int approximateCurveWithCap(const ChainPoint *first, const ChainPoint *last, int cap);

    /// approximates a polygon
    void approximatePolygon(const ChainPoint *cps, const int size, bool polygon);

    /// approximates a rectangle if possible
    void approximateWithCap(const ChainPoint *cps, const int size, bool polygon);

    /// result list
    std::vector<utils::Point32f> approximation;
  };
  } // namespace icl::cv