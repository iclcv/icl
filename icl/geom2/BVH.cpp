// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/BVH.h>
#include <icl/geom2/PointCloud.h>
#include <icl/geom/Camera.h>
#include <icl/geom/ViewRay.h>
#include <algorithm>
#include <cmath>
#include <numeric>

namespace icl::geom2 {

  // --- AABB ---

  struct AABB {
    float min[3] = { 1e30f,  1e30f,  1e30f};
    float max[3] = {-1e30f, -1e30f, -1e30f};

    void expand(const Vec &v) {
      for (int i = 0; i < 3; i++) {
        if (v[i] < min[i]) min[i] = v[i];
        if (v[i] > max[i]) max[i] = v[i];
      }
    }

    void expand(const AABB &o) {
      for (int i = 0; i < 3; i++) {
        if (o.min[i] < min[i]) min[i] = o.min[i];
        if (o.max[i] > max[i]) max[i] = o.max[i];
      }
    }

    Vec centroid() const {
      return Vec((min[0]+max[0])*0.5f, (min[1]+max[1])*0.5f,
                 (min[2]+max[2])*0.5f, 1);
    }

    int longestAxis() const {
      float dx = max[0]-min[0], dy = max[1]-min[1], dz = max[2]-min[2];
      return (dx >= dy && dx >= dz) ? 0 : (dy >= dz ? 1 : 2);
    }

    // Slab-method ray-AABB intersection, returns true if ray hits box
    bool intersectsRay(const Vec &origin, const Vec &invDir, float maxDist) const {
      float t1 = (min[0] - origin[0]) * invDir[0];
      float t2 = (max[0] - origin[0]) * invDir[0];
      float tmin = std::min(t1, t2);
      float tmax = std::max(t1, t2);
      t1 = (min[1] - origin[1]) * invDir[1];
      t2 = (max[1] - origin[1]) * invDir[1];
      tmin = std::max(tmin, std::min(t1, t2));
      tmax = std::min(tmax, std::max(t1, t2));
      t1 = (min[2] - origin[2]) * invDir[2];
      t2 = (max[2] - origin[2]) * invDir[2];
      tmin = std::max(tmin, std::min(t1, t2));
      tmax = std::min(tmax, std::max(t1, t2));
      return tmax >= std::max(tmin, 0.0f) && tmin < maxDist;
    }
  };

  // --- BVH Node ---

  struct BVH::BVHNode {
    AABB bounds;
    int left = -1;       // child index or -1
    int right = -1;
    int triStart = -1;   // leaf: index into triangle array
    int triCount = 0;    // leaf: number of triangles (0 = internal)

    bool isLeaf() const { return triCount > 0; }
  };

  // --- Data ---

  struct BVH::Data {
    std::vector<Triangle> triangles;
    std::vector<AABB> triAABBs;       // per-triangle AABB
    std::vector<Vec> triCentroids;    // per-triangle centroid
    std::vector<BVHNode> nodes;
    std::vector<int> triIndices;      // reordered triangle indices

    static constexpr int MAX_LEAF_SIZE = 4;

    AABB computeTriAABB(int ti) const {
      AABB box;
      const auto &t = triangles[triIndices[ti]];
      box.expand(t.a);
      box.expand(t.b);
      box.expand(t.c);
      return box;
    }

    int buildNode(int start, int end) {
      int idx = (int)nodes.size();
      nodes.push_back({});
      auto &node = nodes[idx];

      // Compute bounds over all triangles in range
      for (int i = start; i < end; i++) {
        node.bounds.expand(triAABBs[triIndices[i]]);
      }

      int count = end - start;
      if (count <= MAX_LEAF_SIZE) {
        // Leaf
        node.triStart = start;
        node.triCount = count;
        return idx;
      }

      // Find split axis and position (median split on centroids)
      AABB centroidBounds;
      for (int i = start; i < end; i++) {
        centroidBounds.expand(triCentroids[triIndices[i]]);
      }
      int axis = centroidBounds.longestAxis();

      int mid = (start + end) / 2;
      std::nth_element(triIndices.begin() + start,
                       triIndices.begin() + mid,
                       triIndices.begin() + end,
                       [&](int a, int b) {
                         return triCentroids[a][axis] < triCentroids[b][axis];
                       });

      // Need to update nodes[idx] after recursive calls since vector may realloc
      int leftIdx = buildNode(start, mid);
      int rightIdx = buildNode(mid, end);
      nodes[idx].left = leftIdx;
      nodes[idx].right = rightIdx;
      nodes[idx].triCount = 0;  // internal node
      return idx;
    }
  };

  // --- Implementation ---

  BVH::BVH() : m_data(std::make_unique<Data>()) {}
  BVH::~BVH() = default;
  BVH::BVH(BVH &&) noexcept = default;
  BVH &BVH::operator=(BVH &&) noexcept = default;

  void BVH::build(std::vector<Triangle> &&triangles) {
    m_data->triangles = std::move(triangles);
    int n = (int)m_data->triangles.size();
    if (n == 0) {
      m_data->nodes.clear();
      return;
    }

    // Pre-compute per-triangle AABBs and centroids
    m_data->triAABBs.resize(n);
    m_data->triCentroids.resize(n);
    for (int i = 0; i < n; i++) {
      auto &t = m_data->triangles[i];
      auto &box = m_data->triAABBs[i];
      box = {};
      box.expand(t.a);
      box.expand(t.b);
      box.expand(t.c);
      m_data->triCentroids[i] = box.centroid();
    }

    // Initialize index array
    m_data->triIndices.resize(n);
    std::iota(m_data->triIndices.begin(), m_data->triIndices.end(), 0);

    // Build tree
    m_data->nodes.clear();
    m_data->nodes.reserve(2 * n);  // upper bound
    m_data->buildNode(0, n);
  }

  BVHHit BVH::intersect(const geom::ViewRay &ray) const {
    if (m_data->nodes.empty()) return {};

    Vec invDir(1.0f / (ray.direction[0] != 0 ? ray.direction[0] : 1e-20f),
               1.0f / (ray.direction[1] != 0 ? ray.direction[1] : 1e-20f),
               1.0f / (ray.direction[2] != 0 ? ray.direction[2] : 1e-20f),
               0);

    float bestDistSq = 1e30f;
    BVHHit result;

    // Stack-based traversal (no recursion)
    int stack[64];
    int top = 0;
    stack[top++] = 0;  // root

    while (top > 0) {
      int ni = stack[--top];
      const auto &node = m_data->nodes[ni];

      if (!node.bounds.intersectsRay(ray.offset, invDir, std::sqrt(bestDistSq))) {
        continue;
      }

      if (node.isLeaf()) {
        // Test triangles in leaf
        for (int i = node.triStart; i < node.triStart + node.triCount; i++) {
          const auto &tri = m_data->triangles[m_data->triIndices[i]];
          Vec ip;
          auto r = ray.getIntersectionWithTriangle(tri.a, tri.b, tri.c, &ip);
          if (r == geom::ViewRay::foundIntersection) {
            Vec d = ip - ray.offset;
            float distSq = d[0]*d[0] + d[1]*d[1] + d[2]*d[2];
            if (distSq < bestDistSq) {
              bestDistSq = distSq;
              result.node = tri.node;
              result.pos = ip;
              result.color = tri.color;
              result.dist = std::sqrt(distSq);
            }
          }
        }
      } else {
        // Push children (push far child first so near child is processed first)
        stack[top++] = node.left;
        stack[top++] = node.right;
      }
    }

    return result;
  }

  void BVH::raycastImage(const geom::Camera &cam, PointCloud &cloud,
                         int stepX, int stepY) const {
    int camW = cam.getResolution().width;
    int camH = cam.getResolution().height;
    int outW = camW / stepX;
    int outH = camH / stepY;

    if (cloud.getDim() != outW * outH) {
      cloud.setSize(utils::Size(outW, outH));
    }

    bool hasColor = cloud.supports(PointCloud::RGBA32f);
    auto xyzSeg = cloud.selectXYZ();
    core::DataSegment<float,4> rgbaSeg;
    if (hasColor) rgbaSeg = cloud.selectRGBA32f();

    #pragma omp parallel for schedule(dynamic, 8) collapse(2)
    for (int y = 0; y < outH; y++) {
      for (int x = 0; x < outW; x++) {
        int idx = y * outW + x;
        float px = x * stepX + stepX * 0.5f;
        float py = y * stepY + stepY * 0.5f;

        geom::ViewRay ray = cam.getViewRay(utils::Point32f(px, py));
        BVHHit hit = intersect(ray);

        auto &xyz = xyzSeg[idx];
        if (hit) {
          xyz[0] = hit.pos[0]; xyz[1] = hit.pos[1]; xyz[2] = hit.pos[2];
          if (hasColor) rgbaSeg[idx] = hit.color;
        } else {
          xyz[0] = xyz[1] = xyz[2] = 0;
          if (hasColor) rgbaSeg[idx] = GeomColor(0,0,0,0);
        }
      }
    }
  }

  int BVH::getTriangleCount() const { return (int)m_data->triangles.size(); }
  int BVH::getNodeCount() const { return (int)m_data->nodes.size(); }

} // namespace icl::geom2
