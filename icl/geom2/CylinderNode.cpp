// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/CylinderNode.h>
#include <cmath>
#include <vector>

namespace icl::geom2 {

  CylinderNode::CylinderNode(float cx, float cy, float cz,
                               float rx, float ry, float h, int steps)
      : m_cx(cx), m_cy(cy), m_cz(cz), m_rx(rx), m_ry(ry), m_h(h), m_steps(steps) {
    generateMesh();
  }

  CylinderNode::~CylinderNode() = default;

  CylinderNode::CylinderNode(const CylinderNode &o) : GeometryNode(o),
      m_cx(o.m_cx), m_cy(o.m_cy), m_cz(o.m_cz),
      m_rx(o.m_rx), m_ry(o.m_ry), m_h(o.m_h), m_steps(o.m_steps) {}

  CylinderNode &CylinderNode::operator=(const CylinderNode &o) {
    if (this != &o) {
      GeometryNode::operator=(o);
      m_cx=o.m_cx; m_cy=o.m_cy; m_cz=o.m_cz;
      m_rx=o.m_rx; m_ry=o.m_ry; m_h=o.m_h; m_steps=o.m_steps;
    }
    return *this;
  }

  CylinderNode::CylinderNode(CylinderNode &&o) noexcept : GeometryNode(std::move(o)),
      m_cx(o.m_cx), m_cy(o.m_cy), m_cz(o.m_cz),
      m_rx(o.m_rx), m_ry(o.m_ry), m_h(o.m_h), m_steps(o.m_steps) {}

  CylinderNode &CylinderNode::operator=(CylinderNode &&o) noexcept {
    if (this != &o) {
      GeometryNode::operator=(std::move(o));
      m_cx=o.m_cx; m_cy=o.m_cy; m_cz=o.m_cz;
      m_rx=o.m_rx; m_ry=o.m_ry; m_h=o.m_h; m_steps=o.m_steps;
    }
    return *this;
  }

  Node *CylinderNode::deepCopy() const { return new CylinderNode(*this); }

  Vec CylinderNode::getCenter() const { return Vec(m_cx, m_cy, m_cz, 1); }
  float CylinderNode::getHeight() const { return m_h; }

  void CylinderNode::setCenter(float cx, float cy, float cz) {
    m_cx=cx; m_cy=cy; m_cz=cz; generateMesh();
  }

  void CylinderNode::setDimensions(float rx, float ry, float h) {
    m_rx=rx; m_ry=ry; m_h=h; generateMesh();
  }

  void CylinderNode::retessellate(int steps) {
    m_steps=steps; generateMesh();
  }

  std::shared_ptr<CylinderNode> CylinderNode::create(float cx, float cy, float cz,
                                                        float rx, float ry, float h,
                                                        int steps) {
    return std::make_shared<CylinderNode>(cx, cy, cz, rx, ry, h, steps);
  }

  void CylinderNode::generateMesh() {
    clearGeometryData();

    float hrx = m_rx / 2, hry = m_ry / 2, hh = m_h / 2;
    int n = m_steps;
    float da = 2.0f * M_PI / n;

    auto &V = vertices();
    auto &N = normals();

    // Vertices: top ring (0..n-1), bottom ring (n..2n-1)
    for (int i = 0; i < n; ++i) {
      float a = i * da;
      float ca = std::cos(a), sa = std::sin(a);
      // top vertex
      V.push_back(Vec(m_cx + hrx*ca, m_cy + hry*sa, m_cz + hh, 1));
      // radial normal for smooth sides
      float nx = ca, ny = sa, nz = 0;
      N.push_back(Vec(nx, ny, nz, 0));
    }
    for (int i = 0; i < n; ++i) {
      float a = i * da;
      float ca = std::cos(a), sa = std::sin(a);
      // bottom vertex
      V.push_back(Vec(m_cx + hrx*ca, m_cy + hry*sa, m_cz - hh, 1));
      float nx = ca, ny = sa, nz = 0;
      N.push_back(Vec(nx, ny, nz, 0));
    }

    // Cap normal indices
    int topNormIdx = (int)N.size();
    N.push_back(Vec(0, 0, 1, 0));  // top cap normal
    int botNormIdx = (int)N.size();
    N.push_back(Vec(0, 0, -1, 0)); // bottom cap normal

    // Side quads
    for (int i = 0; i < n; ++i) {
      int i1 = (i + 1) % n;
      int top0 = i, top1 = i1;
      int bot0 = n + i, bot1 = n + i1;
      quads().push_back({{top0, top1, bot1, bot0}, {top0, top1, bot1, bot0}});
    }

    // Top cap (polygon as triangle fan)
    std::vector<int> topIdx(n), botIdx(n);
    for (int i = 0; i < n; ++i) { topIdx[i] = i; botIdx[i] = n + i; }
    for (int i = 1; i < n - 1; ++i) {
      triangles().push_back({{topIdx[0], topIdx[i], topIdx[i+1]},
                              {topNormIdx, topNormIdx, topNormIdx}});
    }
    // Bottom cap (reversed winding)
    for (int i = 1; i < n - 1; ++i) {
      triangles().push_back({{botIdx[0], botIdx[i+1], botIdx[i]},
                              {botNormIdx, botNormIdx, botNormIdx}});
    }

    setPrimitiveVisible(PrimLine | PrimVertex, false);
  }

} // namespace icl::geom2
