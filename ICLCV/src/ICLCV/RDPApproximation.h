/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/RDPApproximation.h                     **
** Module : ICLCV                                                  **
** Authors: Sergius Gaulik                                         **
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

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Point.h>
#include <ICLUtils/Point32f.h>
#include <ICLCore/Img.h>
#include <ICLCore/Types.h>
#include <vector>

namespace icl{
  namespace cv{

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
    class ICL_CV_API RDPApproximation {

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
  } // namespace cv
}
