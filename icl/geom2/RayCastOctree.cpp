// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/RayCastOctree.h>
#include <icl/geom/ViewRay.h>
#include <icl/utils/Exception.h>
#include <algorithm>

using namespace icl::utils;

namespace icl::geom2 {

  // Squared distance from point to ray (no sqrt needed)
  static inline float sqrRayPointDist(const geom::ViewRay &ray,
                                       const RayCastOctree::Pt &p) {
    float dx = p[0] - ray.offset[0];
    float dy = p[1] - ray.offset[1];
    float dz = p[2] - ray.offset[2];
    float dot = dx*ray.direction[0] + dy*ray.direction[1] + dz*ray.direction[2];
    return (dx*dx + dy*dy + dz*dz) - dot*dot;
  }

  void RayCastOctree::rayCastRec(const Node *n, const geom::ViewRay &ray,
                                  float maxSqrDist, float maxDist,
                                  std::vector<Pt> &result) {
    // Test points in this node
    for (const Pt *p = n->points; p < n->next; ++p) {
      if (sqrRayPointDist(ray, *p) < maxSqrDist) {
        result.push_back(*p);
      }
    }
    // Recurse into children whose bounding sphere intersects the ray corridor
    if (n->children) {
      for (int i = 0; i < 8; ++i) {
        const Node &child = n->children[i];
        if (sqrRayPointDist(ray, child.boundary.center) <=
            sqr(child.radius + maxDist)) {
          rayCastRec(&child, ray, maxSqrDist, maxDist, result);
        }
      }
    }
  }

  std::vector<RayCastOctree::Pt>
  RayCastOctree::rayCast(const geom::ViewRay &ray, float maxDist) const {
    std::vector<Pt> result;
    result.reserve(64);
    rayCastRec(root, ray, sqr(maxDist), maxDist, result);
    return result;
  }

  std::vector<RayCastOctree::Pt>
  RayCastOctree::rayCastSort(const geom::ViewRay &ray, float maxDist) const {
    auto result = rayCast(ray, maxDist);
    const auto &o = ray.offset;
    std::sort(result.begin(), result.end(), [&](const Pt &a, const Pt &b) {
      float da = sqr(a[0]-o[0]) + sqr(a[1]-o[1]) + sqr(a[2]-o[2]);
      float db = sqr(b[0]-o[0]) + sqr(b[1]-o[1]) + sqr(b[2]-o[2]);
      return da < db;
    });
    return result;
  }

  RayCastOctree::Pt
  RayCastOctree::rayCastClosest(const geom::ViewRay &ray, float maxDist) const {
    auto hits = rayCast(ray, maxDist);
    if (hits.empty()) {
      throw ICLException("RayCastOctree::rayCastClosest: no point found");
    }
    const auto &o = ray.offset;
    return *std::min_element(hits.begin(), hits.end(), [&](const Pt &a, const Pt &b) {
      float da = sqr(a[0]-o[0]) + sqr(a[1]-o[1]) + sqr(a[2]-o[2]);
      float db = sqr(b[0]-o[0]) + sqr(b[1]-o[1]) + sqr(b[2]-o[2]);
      return da < db;
    });
  }

} // namespace icl::geom2
