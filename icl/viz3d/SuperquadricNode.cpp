// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/viz3d/SuperquadricNode.h>
#include <cmath>

namespace icl::viz3d {

  // signed power (the superquadric "squareness" parametrisation)
  static inline float cos_sq(float n, float e) {
    const float cn = std::cos(n);
    return (cn < 0 ? -1.f : 1.f) * std::pow(std::fabs(cn), e);
  }
  static inline float sin_sq(float n, float e) {
    const float sn = std::sin(n);
    return (sn < 0 ? -1.f : 1.f) * std::pow(std::fabs(sn), e);
  }

  SuperquadricNode::SuperquadricNode(float cx, float cy, float cz,
                                     float dx, float dy, float dz,
                                     float e1, float e2, int slices, int stacks)
      : m_cx(cx), m_cy(cy), m_cz(cz), m_dx(dx), m_dy(dy), m_dz(dz),
        m_e1(e1), m_e2(e2), m_slices(slices), m_stacks(stacks) {
    generateMesh();
  }

  SuperquadricNode::~SuperquadricNode() = default;

  SuperquadricNode::SuperquadricNode(const SuperquadricNode &o) : GeometryNode(o),
      m_cx(o.m_cx), m_cy(o.m_cy), m_cz(o.m_cz), m_dx(o.m_dx), m_dy(o.m_dy), m_dz(o.m_dz),
      m_e1(o.m_e1), m_e2(o.m_e2), m_slices(o.m_slices), m_stacks(o.m_stacks) {}

  SuperquadricNode &SuperquadricNode::operator=(const SuperquadricNode &o) {
    if (this != &o) {
      GeometryNode::operator=(o);
      m_cx = o.m_cx; m_cy = o.m_cy; m_cz = o.m_cz;
      m_dx = o.m_dx; m_dy = o.m_dy; m_dz = o.m_dz;
      m_e1 = o.m_e1; m_e2 = o.m_e2; m_slices = o.m_slices; m_stacks = o.m_stacks;
    }
    return *this;
  }

  SuperquadricNode::SuperquadricNode(SuperquadricNode &&o) noexcept : GeometryNode(std::move(o)),
      m_cx(o.m_cx), m_cy(o.m_cy), m_cz(o.m_cz), m_dx(o.m_dx), m_dy(o.m_dy), m_dz(o.m_dz),
      m_e1(o.m_e1), m_e2(o.m_e2), m_slices(o.m_slices), m_stacks(o.m_stacks) {}

  SuperquadricNode &SuperquadricNode::operator=(SuperquadricNode &&o) noexcept {
    if (this != &o) {
      GeometryNode::operator=(std::move(o));
      m_cx = o.m_cx; m_cy = o.m_cy; m_cz = o.m_cz;
      m_dx = o.m_dx; m_dy = o.m_dy; m_dz = o.m_dz;
      m_e1 = o.m_e1; m_e2 = o.m_e2; m_slices = o.m_slices; m_stacks = o.m_stacks;
    }
    return *this;
  }

  NodePtr SuperquadricNode::deepCopy() const { return std::make_shared<SuperquadricNode>(*this); }

  Vec SuperquadricNode::getCenter() const { return Vec(m_cx, m_cy, m_cz, 1); }
  Vec SuperquadricNode::getSize() const { return Vec(m_dx, m_dy, m_dz, 1); }
  Vec SuperquadricNode::getExponents() const { return Vec(m_e1, m_e2, 0, 1); }

  void SuperquadricNode::setCenter(float cx, float cy, float cz) {
    m_cx = cx; m_cy = cy; m_cz = cz; generateMesh();
  }
  void SuperquadricNode::setSize(float dx, float dy, float dz) {
    m_dx = dx; m_dy = dy; m_dz = dz; generateMesh();
  }
  void SuperquadricNode::setExponents(float e1, float e2) {
    m_e1 = e1; m_e2 = e2; generateMesh();
  }
  void SuperquadricNode::retessellate(int slices, int stacks) {
    m_slices = slices; m_stacks = stacks; generateMesh();
  }

  std::shared_ptr<SuperquadricNode> SuperquadricNode::create(
      float cx, float cy, float cz, float dx, float dy, float dz,
      float e1, float e2, int slices, int stacks) {
    return std::make_shared<SuperquadricNode>(cx, cy, cz, dx, dy, dz, e1, e2, slices, stacks);
  }

  void SuperquadricNode::generateMesh() {
    clearGeometryData();
    const int na = m_slices, nb = m_stacks;
    const float dAlpha = M_PI / (na - 1);
    const float dBeta = 2 * M_PI / nb;

    auto &V = vertices();
    auto &N = normals();

    for (int i = 0; i < na; ++i) {
      for (int j = 0; j < nb; ++j) {
        float eta = i * dAlpha - (M_PI / 2);
        float omega = j * dBeta - M_PI;

        float X = cos_sq(eta, m_e1) * cos_sq(omega, m_e2);
        float Y = cos_sq(eta, m_e1) * sin_sq(omega, m_e2);
        float Z = sin_sq(eta, m_e1);
        V.push_back(Vec(m_cx + m_dx * X, m_cy + m_dy * Y, m_cz + m_dz * Z, 1));

        float nx = -X, ny = -Y, nz = -Z;
        float len = std::sqrt(nx*nx + ny*ny + nz*nz);
        if (len > 1e-8f) { nx /= len; ny /= len; nz /= len; }
        N.push_back(Vec(nx, ny, nz, 0));

        if (i) {
          int a = j + nb * i;
          int b = j ? (j + nb * i - 1) : (nb - 1) + nb * i;
          int c = j ? (j + nb * (i - 1) - 1) : (nb - 1) + nb * (i - 1);
          int d = j + nb * (i - 1);
          if (i == na - 1) quads().push_back({{a, d, c, b}, {a, d, c, b}, {a, d, c, b}});
          else             quads().push_back({{d, c, b, a}, {d, c, b, a}, {d, c, b, a}});
          if (i != na - 1) lines().push_back({a, b});
          lines().push_back({a, d});
        }
      }
    }
    setPrimitiveVisible(PrimLine | PrimVertex, false);
  }

} // namespace icl::viz3d
