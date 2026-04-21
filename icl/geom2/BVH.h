// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/math/FixedVector.h>
#include <icl/geom2/Primitive.h>  // GeomColor
#include <memory>
#include <vector>

#ifndef ICLGeom2_API
#define ICLGeom2_API
#endif

namespace icl::geom {
  class Camera;
  class ViewRay;
}

namespace icl::geom2 {

  class Node;
  class PointCloud;
  using Vec = math::FixedColVector<float, 4>;

  /// Result of a BVH ray query
  struct BVHHit {
    Node *node = nullptr;
    Vec pos{0,0,0,1};
    GeomColor color{0,0,0,0};
    float dist = -1;
    operator bool() const { return node != nullptr; }
  };

  /// Bounding Volume Hierarchy for fast ray-triangle intersection
  /** Build from a list of world-space triangles, then query with rays.
      Uses median-split construction and stack-based traversal.
      Thread-safe for concurrent queries after build(). */
  class ICLGeom2_API BVH {
  public:
    /// A single triangle with metadata for hit reporting
    struct Triangle {
      Vec a, b, c;          ///< world-space vertices
      Node *node;           ///< owning node (for hit result)
      GeomColor color;      ///< material color (for hit result)
    };

    BVH();
    ~BVH();

    BVH(BVH &&) noexcept;
    BVH &operator=(BVH &&) noexcept;

    /// Build from a list of triangles (moves the data in)
    void build(std::vector<Triangle> &&triangles);

    /// Find the closest intersection along a ray
    BVHHit intersect(const geom::ViewRay &ray) const;

    /// Raycast an entire camera image into a point cloud (OpenMP-parallel)
    /** @param cam    camera to cast from
        @param cloud  target (must support XYZ; RGBA32f written if available)
        @param stepX  pixel step in X (>1 for subsampling)
        @param stepY  pixel step in Y (>1 for subsampling) */
    void raycastImage(const geom::Camera &cam, PointCloud &cloud,
                      int stepX = 1, int stepY = 1) const;

    /// Number of triangles in the BVH
    int getTriangleCount() const;

    /// Number of internal nodes
    int getNodeCount() const;

  private:
    struct BVHNode;
    struct Data;
    std::unique_ptr<Data> m_data;
  };

} // namespace icl::geom2
