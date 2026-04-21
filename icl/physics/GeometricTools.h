// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/geom/GeomDefs.h>
#include <icl/utils/Point32f.h>

namespace icl::physics {
    bool line_segment_intersect(const utils::Point32f &a, const utils::Point32f &b,
                                const utils::Point32f &c, const utils::Point32f &d,
                                utils::Point32f *dst=0,
                                float *dstr=0, float *dsts=0);

    /// Checks whether the given point p lies within the triangle defined by v1,v2 and v3
    bool point_in_triangle(const utils::Point32f &p, const utils::Point32f &v1,
                           const utils::Point32f &v2, const utils::Point32f &v3);
  }