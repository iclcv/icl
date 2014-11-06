#pragma once

#include <ICLGeom/GeomDefs.h>
#include <ICLUtils/Point32f.h>

namespace icl{
  namespace physics{

    bool line_segment_intersect(const utils::Point32f &a, const utils::Point32f &b,
                                const utils::Point32f &c, const utils::Point32f &d,
                                utils::Point32f *dst=0,
                                float *dstr=0, float *dsts=0);

    /// Checks whether the given point p lies within the triangle defined by v1,v2 and v3
    bool point_in_triangle(const utils::Point32f &p, const utils::Point32f &v1,
                           const utils::Point32f &v2, const utils::Point32f &v3);
  }
}
