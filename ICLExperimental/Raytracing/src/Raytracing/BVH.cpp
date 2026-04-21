// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include "BVH.h"
#include <algorithm>
#include <numeric>
#include <cassert>
#include <cstring>

namespace icl::rt {

// ---- Ray-AABB slab test ----

bool BVH::intersectAABB(const BVHRay &ray, const AABB &box, float &tNear, float &tFar) {
  float t1 = (box.min.x - ray.origin.x) * ray.invDir.x;
  float t2 = (box.max.x - ray.origin.x) * ray.invDir.x;
  tNear = std::min(t1, t2);
  tFar  = std::max(t1, t2);

  t1 = (box.min.y - ray.origin.y) * ray.invDir.y;
  t2 = (box.max.y - ray.origin.y) * ray.invDir.y;
  tNear = std::max(tNear, std::min(t1, t2));
  tFar  = std::min(tFar,  std::max(t1, t2));

  t1 = (box.min.z - ray.origin.z) * ray.invDir.z;
  t2 = (box.max.z - ray.origin.z) * ray.invDir.z;
  tNear = std::max(tNear, std::min(t1, t2));
  tFar  = std::min(tFar,  std::max(t1, t2));

  return tNear <= tFar && tFar >= ray.tMin && tNear <= ray.tMax;
}

// ---- Möller-Trumbore ray-triangle intersection ----

bool BVH::intersectTriangle(const BVHRay &ray, const RTFloat3 &v0, const RTFloat3 &v1,
                            const RTFloat3 &v2, float &t, float &u, float &v) {
  RTFloat3 e1 = v1 - v0;
  RTFloat3 e2 = v2 - v0;
  RTFloat3 h = ray.dir.cross(e2);
  float a = e1.dot(h);

  if (a > -1e-8f && a < 1e-8f) return false;

  float f = 1.0f / a;
  RTFloat3 s = ray.origin - v0;
  u = f * s.dot(h);
  if (u < 0.0f || u > 1.0f) return false;

  RTFloat3 q = s.cross(e1);
  v = f * ray.dir.dot(q);
  if (v < 0.0f || u + v > 1.0f) return false;

  t = f * e2.dot(q);
  return t >= ray.tMin && t <= ray.tMax;
}

// ---- BVH construction ----

void BVH::build(const RTVertex *vertices, int numVertices,
                const RTTriangle *triangles, int numTriangles) {
  m_vertices = vertices;
  m_triangles = triangles;
  m_nodes.clear();
  m_triIndices.clear();

  if (numTriangles == 0) return;

  // Precompute centroids and per-triangle AABBs
  m_centroids.resize(numTriangles);
  m_triBounds.resize(numTriangles);
  m_triIndices.resize(numTriangles);
  std::iota(m_triIndices.begin(), m_triIndices.end(), 0);

  for (int i = 0; i < numTriangles; i++) {
    const auto &tri = triangles[i];
    const RTFloat3 &a = vertices[tri.v0].position;
    const RTFloat3 &b = vertices[tri.v1].position;
    const RTFloat3 &c = vertices[tri.v2].position;

    AABB box;
    box.expand(a); box.expand(b); box.expand(c);
    m_triBounds[i] = box;
    m_centroids[i] = box.centroid();
  }

  // Reserve nodes (2*N-1 is the max for a binary tree with N leaves)
  m_nodes.reserve(2 * numTriangles);
  m_nodes.push_back({}); // root node

  buildRecursive(0, 0, numTriangles);

  // Free temporary data
  m_centroids.clear();
  m_triBounds.clear();
}

float BVH::evaluateSAH(uint32_t start, uint32_t count, int axis, float splitPos) const {
  AABB leftBox, rightBox;
  int leftCount = 0, rightCount = 0;

  for (uint32_t i = start; i < start + count; i++) {
    uint32_t ti = m_triIndices[i];
    float c = (axis == 0) ? m_centroids[ti].x : (axis == 1) ? m_centroids[ti].y : m_centroids[ti].z;
    if (c <= splitPos) {
      leftBox.expand(m_triBounds[ti]);
      leftCount++;
    } else {
      rightBox.expand(m_triBounds[ti]);
      rightCount++;
    }
  }

  if (leftCount == 0 || rightCount == 0) return 1e30f;

  return TRAVERSAL_COST + INTERSECT_COST *
    (leftCount * leftBox.surfaceArea() + rightCount * rightBox.surfaceArea());
}

void BVH::buildRecursive(uint32_t nodeIdx, uint32_t start, uint32_t count) {
  // Compute bounds for this node
  AABB bounds;
  for (uint32_t i = start; i < start + count; i++) {
    bounds.expand(m_triBounds[m_triIndices[i]]);
  }
  m_nodes[nodeIdx].bounds = bounds;

  // Leaf node
  if ((int)count <= MAX_LEAF_SIZE) {
    m_nodes[nodeIdx].triOffset = start;
    m_nodes[nodeIdx].triCount = count;
    return;
  }

  // Find best split using SAH with binning
  float bestCost = 1e30f;
  int bestAxis = -1;
  float bestSplit = 0;
  float parentArea = bounds.surfaceArea();

  if (parentArea < 1e-20f) {
    // Degenerate: make a leaf
    m_nodes[nodeIdx].triOffset = start;
    m_nodes[nodeIdx].triCount = count;
    return;
  }

  for (int axis = 0; axis < 3; axis++) {
    float bmin = (axis == 0) ? bounds.min.x : (axis == 1) ? bounds.min.y : bounds.min.z;
    float bmax = (axis == 0) ? bounds.max.x : (axis == 1) ? bounds.max.y : bounds.max.z;

    if (bmax - bmin < 1e-10f) continue;

    for (int b = 1; b < SAH_BINS; b++) {
      float splitPos = bmin + (bmax - bmin) * b / SAH_BINS;
      float cost = evaluateSAH(start, count, axis, splitPos) / parentArea;
      if (cost < bestCost) {
        bestCost = cost;
        bestAxis = axis;
        bestSplit = splitPos;
      }
    }
  }

  // If no good split found, or leaf cost is cheaper, make a leaf
  float leafCost = INTERSECT_COST * count;
  if (bestAxis == -1 || bestCost >= leafCost) {
    m_nodes[nodeIdx].triOffset = start;
    m_nodes[nodeIdx].triCount = count;
    return;
  }

  // Partition triangles
  auto mid = std::partition(
    m_triIndices.begin() + start,
    m_triIndices.begin() + start + count,
    [&](uint32_t ti) {
      float c = (bestAxis == 0) ? m_centroids[ti].x
              : (bestAxis == 1) ? m_centroids[ti].y
              : m_centroids[ti].z;
      return c <= bestSplit;
    });

  uint32_t leftCount = (uint32_t)(mid - (m_triIndices.begin() + start));
  if (leftCount == 0 || leftCount == count) {
    // Partition failed — make a leaf
    m_nodes[nodeIdx].triOffset = start;
    m_nodes[nodeIdx].triCount = count;
    return;
  }

  // Create left child at nodeIdx+1 (implicit in flat BVH layout).
  // Must fully build left subtree before allocating right child
  // so that all left descendants are contiguous.
  uint32_t leftChildIdx = (uint32_t)m_nodes.size();
  m_nodes.push_back({});
  m_nodes[nodeIdx].triCount = 0; // interior node

  buildRecursive(leftChildIdx, start, leftCount);

  // Right child comes after the entire left subtree
  uint32_t rightChildIdx = (uint32_t)m_nodes.size();
  m_nodes.push_back({});
  m_nodes[nodeIdx].rightChildIdx = rightChildIdx;

  buildRecursive(rightChildIdx, start + leftCount, count - leftCount);
}

// ---- BVH traversal ----

BVHHit BVH::trace(const BVHRay &ray) const {
  BVHHit closest;
  if (m_nodes.empty()) return closest;

  // Iterative traversal with stack
  uint32_t stack[64];
  int stackPtr = 0;
  stack[stackPtr++] = 0;

  BVHRay localRay = ray;

  while (stackPtr > 0) {
    uint32_t idx = stack[--stackPtr];
    const auto &node = m_nodes[idx];

    float tNear, tFar;
    if (!intersectAABB(localRay, node.bounds, tNear, tFar)) continue;
    if (tNear > closest.t) continue;

    if (node.triCount > 0) {
      // Leaf: test all triangles
      for (uint32_t i = 0; i < node.triCount; i++) {
        uint32_t ti = m_triIndices[node.triOffset + i];
        const auto &tri = m_triangles[ti];
        float t, u, v;
        if (intersectTriangle(localRay,
              m_vertices[tri.v0].position,
              m_vertices[tri.v1].position,
              m_vertices[tri.v2].position,
              t, u, v)) {
          if (t < closest.t) {
            closest.t = t;
            closest.triIndex = ti;
            closest.u = u;
            closest.v = v;
            localRay.tMax = t;
          }
        }
      }
    } else {
      // Interior: push children (right first so left is processed first)
      if (stackPtr < 63) stack[stackPtr++] = node.rightChildIdx;
      if (stackPtr < 63) stack[stackPtr++] = idx + 1; // left child
    }
  }

  return closest;
}

bool BVH::traceAny(const BVHRay &ray) const {
  if (m_nodes.empty()) return false;

  uint32_t stack[64];
  int stackPtr = 0;
  stack[stackPtr++] = 0;

  while (stackPtr > 0) {
    uint32_t idx = stack[--stackPtr];
    const auto &node = m_nodes[idx];

    float tNear, tFar;
    if (!intersectAABB(ray, node.bounds, tNear, tFar)) continue;

    if (node.triCount > 0) {
      for (uint32_t i = 0; i < node.triCount; i++) {
        uint32_t ti = m_triIndices[node.triOffset + i];
        const auto &tri = m_triangles[ti];
        float t, u, v;
        if (intersectTriangle(ray,
              m_vertices[tri.v0].position,
              m_vertices[tri.v1].position,
              m_vertices[tri.v2].position,
              t, u, v)) {
          return true;
        }
      }
    } else {
      if (stackPtr < 63) stack[stackPtr++] = node.rightChildIdx;
      if (stackPtr < 63) stack[stackPtr++] = idx + 1;
    }
  }

  return false;
}

} // namespace icl::rt
