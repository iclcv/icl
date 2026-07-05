// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/viz3d/GeometryNode.h>
#include <memory>

namespace icl::viz3d {

  /// Parametric superquadric — params are source of truth, mesh is derived
  /** Inherits GeometryNode (NOT MeshNode) — no mutable vertex access. The shape
      is built in local coordinates centred at (cx,cy,cz); orient it with the
      node transform (rotate/setTransformation), not a baked-in rotation.

      The two "squareness" exponents e1 (north-south) and e2 (east-west) blend
      the surface between a sphere (e1=e2=1), a box (e→0), and a pinched/star
      shape (e>1). Use setExponents/setSize to morph; the mesh regenerates. */
  class ICLViz3d_API SuperquadricNode : public GeometryNode {
  public:
    SuperquadricNode(float cx, float cy, float cz,
                     float dx, float dy, float dz,
                     float e1 = 1, float e2 = 1,
                     int slices = 30, int stacks = 30);
    ~SuperquadricNode() override;
    SuperquadricNode(const SuperquadricNode &);
    SuperquadricNode &operator=(const SuperquadricNode &);
    SuperquadricNode(SuperquadricNode &&) noexcept;
    SuperquadricNode &operator=(SuperquadricNode &&) noexcept;
    NodePtr deepCopy() const override;

    Vec getCenter() const;
    Vec getSize() const;
    Vec getExponents() const;     // (e1, e2, 0, 1)

    void setCenter(float cx, float cy, float cz);
    void setSize(float dx, float dy, float dz);
    void setExponents(float e1, float e2);
    void retessellate(int slices, int stacks);

    static std::shared_ptr<SuperquadricNode> create(
        float cx, float cy, float cz, float dx, float dy, float dz,
        float e1 = 1, float e2 = 1, int slices = 30, int stacks = 30);

  private:
    float m_cx, m_cy, m_cz, m_dx, m_dy, m_dz, m_e1, m_e2;
    int m_slices, m_stacks;
    void generateMesh();
  };

} // namespace icl::viz3d
