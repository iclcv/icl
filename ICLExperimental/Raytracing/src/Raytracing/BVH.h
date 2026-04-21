// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include "RaytracerTypes.h"
#include <vector>
#include <limits>
#include <cmath>

namespace icl::rt {

/// Axis-aligned bounding box.
struct AABB {
  RTFloat3 min{1e30f, 1e30f, 1e30f};
  RTFloat3 max{-1e30f, -1e30f, -1e30f};

  void expand(const RTFloat3 &p) {
    if (p.x < min.x) min.x = p.x; if (p.x > max.x) max.x = p.x;
    if (p.y < min.y) min.y = p.y; if (p.y > max.y) max.y = p.y;
    if (p.z < min.z) min.z = p.z; if (p.z > max.z) max.z = p.z;
  }

  void expand(const AABB &other) {
    expand(other.min);
    expand(other.max);
  }

  float surfaceArea() const {
    RTFloat3 d = max - min;
    return 2.0f * (d.x * d.y + d.y * d.z + d.z * d.x);
  }

  RTFloat3 centroid() const {
    return {(min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f, (min.z + max.z) * 0.5f};
  }

  int longestAxis() const {
    RTFloat3 d = max - min;
    if (d.x >= d.y && d.x >= d.z) return 0;
    if (d.y >= d.z) return 1;
    return 2;
  }
};

/// Flat BVH node. Left child is at index+1; right child at rightChildIdx.
/// Leaf nodes have triCount > 0 and store triangles at triOffset.
struct BVHNode {
  AABB bounds;
  uint32_t rightChildIdx = 0; // 0 for leaf nodes
  uint32_t triOffset = 0;
  uint32_t triCount = 0;
  uint32_t _pad = 0;
};

/// Ray for BVH traversal (origin + direction + precomputed inverse direction).
struct BVHRay {
  RTFloat3 origin;
  RTFloat3 dir;
  RTFloat3 invDir;
  float tMin = 0.001f;
  float tMax = 1e30f;
};

/// Result of a ray-BVH intersection.
struct BVHHit {
  float t = 1e30f;
  uint32_t triIndex = ~0u;
  float u = 0, v = 0; // barycentric coordinates
  bool hit() const { return triIndex != ~0u; }
};

/// BVH acceleration structure for a set of triangles.
/// Uses SAH (Surface Area Heuristic) for construction.
class BVH {
public:
  BVH() = default;

  /// Build BVH from triangles and vertices.
  void build(const RTVertex *vertices, int numVertices,
             const RTTriangle *triangles, int numTriangles);

  /// Trace a ray through the BVH, returning the closest hit.
  BVHHit trace(const BVHRay &ray) const;

  /// Test if any intersection exists (for shadow rays). Stops at first hit.
  bool traceAny(const BVHRay &ray) const;

  /// Get the bounding box of the entire BVH.
  const AABB &getBounds() const { return m_nodes.empty() ? m_emptyBounds : m_nodes[0].bounds; }

  bool empty() const { return m_nodes.empty(); }

  /// Access the flat node array (for GPU upload).
  const std::vector<BVHNode> &getNodes() const { return m_nodes; }

  /// Access the permuted triangle index array (for GPU upload).
  const std::vector<uint32_t> &getTriIndices() const { return m_triIndices; }

private:
  void buildRecursive(uint32_t nodeIdx, uint32_t start, uint32_t count);
  float evaluateSAH(uint32_t start, uint32_t count, int axis, float splitPos) const;

  /// Ray-AABB slab test. Returns (tMin, tMax) or tMin > tMax if no hit.
  static bool intersectAABB(const BVHRay &ray, const AABB &box, float &tNear, float &tFar);

  /// Möller-Trumbore ray-triangle intersection.
  static bool intersectTriangle(const BVHRay &ray, const RTFloat3 &v0, const RTFloat3 &v1,
                                const RTFloat3 &v2, float &t, float &u, float &v);

  std::vector<BVHNode> m_nodes;
  std::vector<uint32_t> m_triIndices; // permuted triangle indices
  const RTVertex *m_vertices = nullptr;
  const RTTriangle *m_triangles = nullptr;
  std::vector<RTFloat3> m_centroids;
  std::vector<AABB> m_triBounds;
  AABB m_emptyBounds;

  static constexpr int MAX_LEAF_SIZE = 4;
  static constexpr int SAH_BINS = 12;
  static constexpr float TRAVERSAL_COST = 1.0f;
  static constexpr float INTERSECT_COST = 1.0f;
};

} // namespace icl::rt
