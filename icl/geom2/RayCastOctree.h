// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/math/Octree.h>
#include <icl/math/FixedVector.h>
#include <vector>

#ifndef ICLGeom2_API
#define ICLGeom2_API
#endif

namespace icl::geom {
  struct ViewRay;
}

namespace icl::geom2 {

  /// Octree with accelerated ray-cast queries
  /** Wraps math::Octree<float> and adds rayCast methods that exploit the
      octree's bounding-sphere hierarchy for fast ray-to-point proximity
      queries. Each point stores (x, y, z, userData) where userData
      (the w-component) can carry an index or ID.

      Thread-safe for concurrent reads after construction + insertion. */
  class ICLGeom2_API RayCastOctree : public math::Octree<float, 32> {
    using Super = math::Octree<float, 32>;
  public:
    using Super::Super;  // inherit constructors
    using Pt = math::FixedColVector<float, 4>;

    /// Find all points closer than maxDist to the ray
    std::vector<Pt> rayCast(const geom::ViewRay &ray, float maxDist = 1) const;

    /// Like rayCast, but results sorted by distance to ray origin
    std::vector<Pt> rayCastSort(const geom::ViewRay &ray, float maxDist = 1) const;

    /// Find the single point closest to the ray origin (within maxDist of ray)
    /** Throws ICLException if no point found. */
    Pt rayCastClosest(const geom::ViewRay &ray, float maxDist = 1) const;

  private:
    static void rayCastRec(const Node *n, const geom::ViewRay &ray,
                           float maxSqrDist, float maxDist,
                           std::vector<Pt> &result);
  };

} // namespace icl::geom2
