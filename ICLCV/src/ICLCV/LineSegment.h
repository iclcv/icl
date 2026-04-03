// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Erik Weitnauer

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <iostream>

namespace icl::cv {
  /// line segment class (e.g. used for run-length coding)
  /** A line segment represents a set of successive pixels in one
      image line that have the same value.
      Note: the LineSegment's xend location is the first pixel
            after the LineSegment, that does not have the same value
  */
  struct ICLCV_API LineSegment{
    int x;    //!<first pixel of this line segment
    int y;    //!<y position in the image of this line segment
    int xend; //!<first pixel AFTER this line segment

    /// creates an empty uninitialized line segment
    LineSegment(){}

    /// creates a line segment with given parameters
    LineSegment(int x, int y, int xend):
      x(x),y(y),xend(xend){}

    /// computes the line segments length (xend-x)
    int len() const { return xend-x; }
  };


  /// ostream operator for the line-segment type
  inline std::ostream &operator<<(std::ostream &s, const LineSegment &l){
    return s << "LineSegment(x="<<l.x<<" y="<<l.y<<" len="<<l.len()<<")";
  }

  } // namespace icl::cv