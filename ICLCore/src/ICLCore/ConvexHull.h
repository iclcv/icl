// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Point.h>
#include <ICLUtils/Point32f.h>
#include <vector>

namespace icl{
  namespace core{

    /// convex hull monotone chain algorithm for int-points
    /** @param P list of utils::Point (input) call-by-value, as we need an inplace-sort
                 internally
        @return list of points of the convex hull first point is identical
        to the last point in this list!
    */
    ICLCore_API std::vector<utils::Point> convexHull(std::vector<utils::Point> P);

    /// convex hull monotone chain algorithm for float-points
    /** @param P list of utils::Point32f (input) call-by-value, as we need an inplace-sort
                 internally
        @return list of points of the convex hull first point is identical
        to the last point in this list!
    */
    ICLCore_API std::vector<utils::Point32f> convexHull(std::vector<utils::Point32f> P);

  } // namespace geom
}
