// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/geom/GeomDefs.h>
#include <icl/utils/Exception.h>
#include <icl/utils/Point32f.h>
#include <iostream>

namespace icl::geom {
  /** \cond */
  struct PlaneEquation;
  /** \endcond */

  /// This is a view-ray's line equation in parameter form
  /** A view-ray is described by the equation
      \f[ V: \mbox{offset} + \lambda \cdot \mbox{direction} \f]
  */
  struct ICLGeom_API ViewRay{
    /// Constructor with given offset and direction vector
    explicit ViewRay(const Vec &offset=Vec(), const Vec &direction=Vec(), bool autoNormalizeDirection=false);

    /// line offset
    Vec offset;

    /// line direction
    Vec direction;

    /// calculates line-plane intersection
    /** @see static Camera::getIntersection function */
    Vec getIntersection(const PlaneEquation &plane) const;

    /// ray-triangle intersection results
    enum TriangleIntersection{
      noIntersection,
      foundIntersection,
      wrongDirection,
      degenerateTriangle,
      rayIsCollinearWithTriangle
    };

    /// calculates intersection with given triangle
    /** inspired by http://softsurfer.com/Archive/algorithm_0105/algorithm_0105.htm#intersect_RayTriangle()

        The actual intersection point in 3D can optionally be stored into a non-null "intersectionPoint"
        parameter. The parametric triangle coordinats can also optionally be stored in a non-null
        "parametricCoords" parameter. "parametricCoords" is only written if it is not null and if
        the returned intersection result is "foundIntersection"
        */
    TriangleIntersection getIntersectionWithTriangle(const geom::Vec &a, const geom::Vec &b, const geom::Vec &c,
                                                     geom::Vec *intersectionPoint=0,
                                                     utils::Point32f *parametricCoords=0) const;

    /// calculates the closest distance to the given 3D-Point
    /** for following formula is used:
        <pre>
        ViewRay: o + lambda * v;
        3D-point: p

        distance = sqrt( |p-o|^2 - |(p-o).v|^2 )
        </pre>
    */
    float closestDistanceTo(const Vec &p) const;

    /// closest squared distance
    float closestSqrDistanceTo(const Vec &p) const;

    /// calculates the closes distance to the given other ViewRay
    /** Here, we use the following formula:
        <pre>
        this ViewRay:  o + lambda * v
        other ViewRay: k + beta * m

        distance = | (k-o).(v x m) |
                   -----------------
                       | v x m |
        </pre>
    */
    float closestDistanceTo(const ViewRay &other) const;

    /// evaluates ViewRay' line equation at given lambda
    /** formula : offset + lambda * direction */
    Vec operator()(float lambda) const;
  };

  /// ostream operator
  ICLGeom_API std::ostream &operator<<(std::ostream &s, const ViewRay &vr);
  } // namespace icl::geom