// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Point32f.h>
#include <icl/utils/Exception.h>
#include <icl/math/FixedVector.h>

namespace icl::math {
  /// A straight line is parameterized in offset/direction form
  /** This formular is used:
      \f[ L(x) = \vec{o} + x\vec{v} \f]

      The template is instantiated for template parameter Pos type
      Point32f and FixedColVector<float,2>
  */
  struct ICLMath_API StraightLine2D{
    /// internal typedef
    using PointPolar = FixedColVector<float,2>;

    /// internal typedef for 2D points
    using Pos = FixedColVector<float,2>;

    /// creates a straight line from given angle and distance to origin
    StraightLine2D(float angle, float distance);

    /// creates a straight line from given 2 points
    StraightLine2D(const Pos &o=Pos(0,0), const Pos &v=Pos(0,0));

    /// creates a straight line from given point32f
    StraightLine2D(const utils::Point32f &o, const utils::Point32f &v);

    /// 2D offset vector
    Pos o;

    /// 2D direction vector
    Pos v;

    /// computes closest distance to given 2D point
    float distance(const Pos &p) const;

    /// computes closest distance to given 2D point
    /* result is positive if p is left of this->v
        and negative otherwise */
    float signedDistance(const Pos &p) const;

    /// computes closest distance to given 2D point
    inline float distance(const utils::Point32f &p) const {
      return distance(Pos(p.x,p.y));
    }

    /// computes closest distance to given 2D point
    /* result is positive if p is left of this->v
        and negative otherwise */
    float signedDistance(const utils::Point32f &p) const {
      return signedDistance(Pos(p.x,p.y));
    }

    /// computes intersection with given other straight line
    /** if lines are parallel, an ICLException is thrown */
    Pos intersect(const StraightLine2D &o) const;

    /// returns current angle and distance
    PointPolar getAngleAndDistance() const;

    /// retunrs the closest point on the straight line to a given other point
    Pos getClosestPoint(const Pos &p) const;
  };
  } // namespace icl::math