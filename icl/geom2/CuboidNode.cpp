// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/CuboidNode.h>

namespace icl::geom2 {

  CuboidNode::CuboidNode(float cx, float cy, float cz, float dx, float dy, float dz)
      : m_cx(cx), m_cy(cy), m_cz(cz), m_dx(dx), m_dy(dy), m_dz(dz) {
    generateMesh();
  }

  CuboidNode::CuboidNode(float cx, float cy, float cz, float size)
      : CuboidNode(cx, cy, cz, size, size, size) {}

  CuboidNode::~CuboidNode() = default;

  CuboidNode::CuboidNode(const CuboidNode &o) : GeometryNode(o),
      m_cx(o.m_cx), m_cy(o.m_cy), m_cz(o.m_cz),
      m_dx(o.m_dx), m_dy(o.m_dy), m_dz(o.m_dz) {}

  CuboidNode &CuboidNode::operator=(const CuboidNode &o) {
    if (this != &o) {
      GeometryNode::operator=(o);
      m_cx = o.m_cx; m_cy = o.m_cy; m_cz = o.m_cz;
      m_dx = o.m_dx; m_dy = o.m_dy; m_dz = o.m_dz;
    }
    return *this;
  }

  CuboidNode::CuboidNode(CuboidNode &&o) noexcept : GeometryNode(std::move(o)),
      m_cx(o.m_cx), m_cy(o.m_cy), m_cz(o.m_cz),
      m_dx(o.m_dx), m_dy(o.m_dy), m_dz(o.m_dz) {}

  CuboidNode &CuboidNode::operator=(CuboidNode &&o) noexcept {
    if (this != &o) {
      GeometryNode::operator=(std::move(o));
      m_cx = o.m_cx; m_cy = o.m_cy; m_cz = o.m_cz;
      m_dx = o.m_dx; m_dy = o.m_dy; m_dz = o.m_dz;
    }
    return *this;
  }

  Node *CuboidNode::deepCopy() const { return new CuboidNode(*this); }

  Vec CuboidNode::getCenter() const { return Vec(m_cx, m_cy, m_cz, 1); }
  Vec CuboidNode::getExtents() const { return Vec(m_dx, m_dy, m_dz, 1); }

  void CuboidNode::setCenter(float cx, float cy, float cz) {
    m_cx = cx; m_cy = cy; m_cz = cz; generateMesh();
  }

  void CuboidNode::setExtents(float dx, float dy, float dz) {
    m_dx = dx; m_dy = dy; m_dz = dz; generateMesh();
  }

  std::shared_ptr<CuboidNode> CuboidNode::create(float cx, float cy, float cz,
                                                   float dx, float dy, float dz) {
    return std::make_shared<CuboidNode>(cx, cy, cz, dx, dy, dz);
  }

  std::shared_ptr<CuboidNode> CuboidNode::createCube(float cx, float cy, float cz, float size) {
    return std::make_shared<CuboidNode>(cx, cy, cz, size);
  }

  void CuboidNode::generateMesh() {
    clearGeometryData();

    float x = m_cx, y = m_cy, z = m_cz;
    float dx = m_dx / 2, dy = m_dy / 2, dz = m_dz / 2;

    auto &V = vertices();
    V.push_back(Vec(x+dx, y+dy, z-dz, 1));  // 0
    V.push_back(Vec(x-dx, y+dy, z-dz, 1));  // 1
    V.push_back(Vec(x-dx, y-dy, z-dz, 1));  // 2
    V.push_back(Vec(x+dx, y-dy, z-dz, 1));  // 3
    V.push_back(Vec(x+dx, y+dy, z+dz, 1));  // 4
    V.push_back(Vec(x-dx, y+dy, z+dz, 1));  // 5
    V.push_back(Vec(x-dx, y-dy, z+dz, 1));  // 6
    V.push_back(Vec(x+dx, y-dy, z+dz, 1));  // 7

    // 6 face normals
    auto &N = normals();
    N.push_back(Vec(0, 0, -1, 0));  // 0: -Z face
    N.push_back(Vec(0, 0, 1, 0));   // 1: +Z face
    N.push_back(Vec(0, 1, 0, 0));   // 2: +Y face
    N.push_back(Vec(0, -1, 0, 0));  // 3: -Y face
    N.push_back(Vec(1, 0, 0, 0));   // 4: +X face
    N.push_back(Vec(-1, 0, 0, 0));  // 5: -X face

    // 6 faces as quads (vertex indices + normal indices)
    quads().push_back({{0,1,2,3}, {0,0,0,0}});  // -Z
    quads().push_back({{4,7,6,5}, {1,1,1,1}});  // +Z
    quads().push_back({{0,4,5,1}, {2,2,2,2}});  // +Y
    quads().push_back({{3,2,6,7}, {3,3,3,3}});  // -Y
    quads().push_back({{0,3,7,4}, {4,4,4,4}});  // +X
    quads().push_back({{1,5,6,2}, {5,5,5,5}});  // -X

    // 12 edges
    auto &L = lines();
    L.push_back({0,1}); L.push_back({1,2}); L.push_back({2,3}); L.push_back({3,0});
    L.push_back({4,5}); L.push_back({5,6}); L.push_back({6,7}); L.push_back({7,4});
    L.push_back({0,4}); L.push_back({1,5}); L.push_back({2,6}); L.push_back({3,7});

    setPrimitiveVisible(PrimLine | PrimVertex, false);
  }

} // namespace icl::geom2
