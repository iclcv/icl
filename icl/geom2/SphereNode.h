// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/geom2/GeometryNode.h>
#include <memory>

namespace icl::geom2 {

  /// Parametric sphere/spheroid — params are source of truth, mesh is derived
  /** Inherits GeometryNode (NOT MeshNode) — no mutable vertex access.
      Use setCenter/setRadius/setRadii to change shape; mesh regenerates automatically. */
  class ICLGeom2_API SphereNode : public GeometryNode {
  public:
    SphereNode(float cx, float cy, float cz, float radius,
               int slices = 30, int stacks = 30);
    SphereNode(float cx, float cy, float cz,
               float rx, float ry, float rz,
               int slices = 30, int stacks = 30);
    ~SphereNode() override;
    SphereNode(const SphereNode &other);
    SphereNode &operator=(const SphereNode &other);
    SphereNode(SphereNode &&other) noexcept;
    SphereNode &operator=(SphereNode &&other) noexcept;
    Node *deepCopy() const override;

    Vec getCenter() const;
    Vec getRadii() const;
    float getRadius() const;

    void setCenter(float cx, float cy, float cz);
    void setRadius(float r);
    void setRadii(float rx, float ry, float rz);
    void retessellate(int slices, int stacks);

    static std::shared_ptr<SphereNode> create(float cx, float cy, float cz, float r,
                                               int slices = 30, int stacks = 30);

  private:
    float m_cx, m_cy, m_cz, m_rx, m_ry, m_rz;
    int m_slices, m_stacks;
    void generateMesh();
  };

} // namespace icl::geom2
