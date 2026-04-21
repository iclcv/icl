// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Robert Haschke

#include <icl/utils/Point.h>
#include <icl/utils/Point32f.h>
#include <cmath>

namespace icl::utils {
  const Point Point::null(0,0);

  float Point::distanceTo(const Point &p) const{
    return sqrt(pow(static_cast<float>(p.x-x), 2) + pow(static_cast<float>(p.y-y), 2));
  }

  Point::Point(const Point32f &p){
    x = static_cast<int>(::round(p.x));
    y = static_cast<int>(::round(p.y));
  }

  Point Point::transform(double xfac, double yfac) const {
    return Point(static_cast<int>(xfac * x), static_cast<int>(yfac * y));
  }

  std::ostream &operator<<(std::ostream &s, const Point &p){
    return s << "(" << p.x << ',' << p.y << ")";
  }

  std::istream &operator>>(std::istream &s, Point &p){
    Point32f p32;
    s >> p32;
    p = Point(p32);
    return s;
  }


  } // namespace icl::utils