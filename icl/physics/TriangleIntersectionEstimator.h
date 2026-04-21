// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/geom/ViewRay.h>
#include <icl/utils/Point32f.h>

namespace icl::physics {
class ICLPhysics_API TriangleIntersectionEstimator{
  public:
  enum IntersectionType{
    noIntersection,
    foundIntersection,
    wrongDirection,
    degenerateTriangle,
    rayIsCollinearWithTriangle
  };

  struct ICLPhysics_API Triangle{
    Triangle(){}
    Triangle(const geom::Vec &a, const geom::Vec &b, const geom::Vec &c):a(a),b(b),c(c){}
    geom::Vec a,b,c;
  };


  struct ICLPhysics_API Intersection{
      Intersection(IntersectionType type=noIntersection,
                   const geom::Vec &position=geom::Vec(),
                   const utils::Point32f &trianglePosition=utils::Point32f::null):
      type(type),position(position),trianglePosition(trianglePosition){}
    IntersectionType type;
    geom::Vec position;
    utils::Point32f trianglePosition;
    operator bool() const { return type == foundIntersection; }
  };

  static Intersection find(const Triangle &t, const geom::ViewRay &r);
};

}