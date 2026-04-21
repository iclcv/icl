// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/ConeNode.h>
#include <cmath>

namespace icl::geom2 {

  ConeNode::ConeNode(float cx, float cy, float cz,
                       float rx, float ry, float h, int steps)
      : m_cx(cx), m_cy(cy), m_cz(cz), m_rx(rx), m_ry(ry), m_h(h), m_steps(steps) {
    generateMesh();
  }

  ConeNode::~ConeNode() = default;

  ConeNode::ConeNode(const ConeNode &o) : GeometryNode(o),
      m_cx(o.m_cx), m_cy(o.m_cy), m_cz(o.m_cz),
      m_rx(o.m_rx), m_ry(o.m_ry), m_h(o.m_h), m_steps(o.m_steps) {}

  ConeNode &ConeNode::operator=(const ConeNode &o) {
    if (this != &o) {
      GeometryNode::operator=(o);
      m_cx=o.m_cx; m_cy=o.m_cy; m_cz=o.m_cz;
      m_rx=o.m_rx; m_ry=o.m_ry; m_h=o.m_h; m_steps=o.m_steps;
    }
    return *this;
  }

  ConeNode::ConeNode(ConeNode &&o) noexcept : GeometryNode(std::move(o)),
      m_cx(o.m_cx), m_cy(o.m_cy), m_cz(o.m_cz),
      m_rx(o.m_rx), m_ry(o.m_ry), m_h(o.m_h), m_steps(o.m_steps) {}

  ConeNode &ConeNode::operator=(ConeNode &&o) noexcept {
    if (this != &o) {
      GeometryNode::operator=(std::move(o));
      m_cx=o.m_cx; m_cy=o.m_cy; m_cz=o.m_cz;
      m_rx=o.m_rx; m_ry=o.m_ry; m_h=o.m_h; m_steps=o.m_steps;
    }
    return *this;
  }

  Node *ConeNode::deepCopy() const { return new ConeNode(*this); }

  Vec ConeNode::getCenter() const { return Vec(m_cx, m_cy, m_cz, 1); }
  float ConeNode::getHeight() const { return m_h; }

  void ConeNode::setCenter(float cx, float cy, float cz) {
    m_cx=cx; m_cy=cy; m_cz=cz; generateMesh();
  }

  void ConeNode::setDimensions(float rx, float ry, float h) {
    m_rx=rx; m_ry=ry; m_h=h; generateMesh();
  }

  void ConeNode::retessellate(int steps) {
    m_steps=steps; generateMesh();
  }

  std::shared_ptr<ConeNode> ConeNode::create(float cx, float cy, float cz,
                                               float rx, float ry, float h,
                                               int steps) {
    return std::make_shared<ConeNode>(cx, cy, cz, rx, ry, h, steps);
  }

  void ConeNode::generateMesh() {
    clearGeometryData();

    float hrx = m_rx / 2, hry = m_ry / 2, hh = m_h / 2;
    int n = m_steps;
    float da = 2.0f * M_PI / n;

    auto &V = vertices();
    auto &N = normals();

    // Apex (vertex 0)
    V.push_back(Vec(m_cx, m_cy, m_cz + hh, 1));
    N.push_back(Vec(0, 0, 1, 0));

    // Base ring (vertices 1..n)
    for (int i = 0; i < n; ++i) {
      float a = i * da;
      V.push_back(Vec(m_cx + hrx * std::cos(a), m_cy + hry * std::sin(a), m_cz - hh, 1));
      // Approximate side normal
      float nx = std::cos(a), ny = std::sin(a);
      float slope = std::sqrt(hrx*hrx + hry*hry) / m_h;
      float nz = slope;
      float len = std::sqrt(nx*nx + ny*ny + nz*nz);
      N.push_back(Vec(nx/len, ny/len, nz/len, 0));
    }

    // Bottom cap normal
    int botNormIdx = (int)N.size();
    N.push_back(Vec(0, 0, -1, 0));

    // Side triangles: apex(0) → base[i] → base[i+1]
    for (int i = 0; i < n; ++i) {
      int i1 = 1 + i;
      int i2 = 1 + (i + 1) % n;
      triangles().push_back({{0, i1, i2}, {0, i1, i2}});
    }

    // Base cap (triangle fan, reversed winding)
    for (int i = 1; i < n - 1; ++i) {
      triangles().push_back({{1, 1 + i + 1, 1 + i},
                              {botNormIdx, botNormIdx, botNormIdx}});
    }

    setPrimitiveVisible(PrimLine | PrimVertex, false);
  }

} // namespace icl::geom2
