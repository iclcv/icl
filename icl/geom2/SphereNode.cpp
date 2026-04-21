// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/SphereNode.h>
#include <cmath>

namespace icl::geom2 {

  SphereNode::SphereNode(float cx, float cy, float cz, float radius, int slices, int stacks)
      : m_cx(cx), m_cy(cy), m_cz(cz), m_rx(radius), m_ry(radius), m_rz(radius),
        m_slices(slices), m_stacks(stacks) {
    generateMesh();
  }

  SphereNode::SphereNode(float cx, float cy, float cz, float rx, float ry, float rz,
                         int slices, int stacks)
      : m_cx(cx), m_cy(cy), m_cz(cz), m_rx(rx), m_ry(ry), m_rz(rz),
        m_slices(slices), m_stacks(stacks) {
    generateMesh();
  }

  SphereNode::~SphereNode() = default;

  SphereNode::SphereNode(const SphereNode &other) : GeometryNode(other),
      m_cx(other.m_cx), m_cy(other.m_cy), m_cz(other.m_cz),
      m_rx(other.m_rx), m_ry(other.m_ry), m_rz(other.m_rz),
      m_slices(other.m_slices), m_stacks(other.m_stacks) {}

  SphereNode &SphereNode::operator=(const SphereNode &other) {
    if (this != &other) {
      GeometryNode::operator=(other);
      m_cx = other.m_cx; m_cy = other.m_cy; m_cz = other.m_cz;
      m_rx = other.m_rx; m_ry = other.m_ry; m_rz = other.m_rz;
      m_slices = other.m_slices; m_stacks = other.m_stacks;
    }
    return *this;
  }

  SphereNode::SphereNode(SphereNode &&other) noexcept : GeometryNode(std::move(other)),
      m_cx(other.m_cx), m_cy(other.m_cy), m_cz(other.m_cz),
      m_rx(other.m_rx), m_ry(other.m_ry), m_rz(other.m_rz),
      m_slices(other.m_slices), m_stacks(other.m_stacks) {}

  SphereNode &SphereNode::operator=(SphereNode &&other) noexcept {
    if (this != &other) {
      GeometryNode::operator=(std::move(other));
      m_cx = other.m_cx; m_cy = other.m_cy; m_cz = other.m_cz;
      m_rx = other.m_rx; m_ry = other.m_ry; m_rz = other.m_rz;
      m_slices = other.m_slices; m_stacks = other.m_stacks;
    }
    return *this;
  }

  Node *SphereNode::deepCopy() const { return new SphereNode(*this); }

  Vec SphereNode::getCenter() const { return Vec(m_cx, m_cy, m_cz, 1); }
  Vec SphereNode::getRadii() const { return Vec(m_rx, m_ry, m_rz, 1); }
  float SphereNode::getRadius() const { return m_rx; }

  void SphereNode::setCenter(float cx, float cy, float cz) {
    m_cx = cx; m_cy = cy; m_cz = cz;
    generateMesh();
  }

  void SphereNode::setRadius(float r) {
    m_rx = m_ry = m_rz = r;
    generateMesh();
  }

  void SphereNode::setRadii(float rx, float ry, float rz) {
    m_rx = rx; m_ry = ry; m_rz = rz;
    generateMesh();
  }

  void SphereNode::retessellate(int slices, int stacks) {
    m_slices = slices; m_stacks = stacks;
    generateMesh();
  }

  std::shared_ptr<SphereNode> SphereNode::create(float cx, float cy, float cz, float r,
                                                   int slices, int stacks) {
    return std::make_shared<SphereNode>(cx, cy, cz, r, slices, stacks);
  }

  void SphereNode::generateMesh() {
    clearGeometryData();

    const int na = m_slices;
    const int nb = m_stacks;
    const float dAlpha = 2.0f * M_PI / na;
    const float dBeta = M_PI / (nb - 1);

    auto &V = vertices();
    auto &N = normals();

    for (int j = 0; j < nb; ++j) {
      for (int i = 0; i < na; ++i) {
        float alpha = i * dAlpha;
        float beta = j * dBeta;
        float ca = std::cos(alpha), sa = std::sin(alpha);
        float cb = std::cos(beta), sb = std::sin(beta);

        V.push_back(Vec(m_cx + m_rx * ca * sb,
                        m_cy + m_ry * sa * sb,
                        m_cz + m_rz * cb, 1));

        float nx = ca * sb, ny = sa * sb, nz = cb;
        float len = std::sqrt(nx*nx + ny*ny + nz*nz);
        if (len > 1e-8f) { nx /= len; ny /= len; nz /= len; }
        N.push_back(Vec(nx, ny, nz, 0));

        if (j > 0) {
          int a = i + na * j;
          int b = (i ? (i - 1) : (na - 1)) + na * j;
          int c = (i ? (i - 1) : (na - 1)) + na * (j - 1);
          int d = i + na * (j - 1);

          if (j == nb - 1) {
            quads().push_back({{a, d, c, b}, {a, d, c, b}});
          } else {
            quads().push_back({{d, c, b, a}, {d, c, b, a}});
          }

          // Edge lines
          lines().push_back({a, b});
          lines().push_back({a, d});
        }
      }
    }

    // Hide wireframe by default (solid sphere)
    setPrimitiveVisible(PrimLine | PrimVertex, false);
  }

} // namespace icl::geom2
