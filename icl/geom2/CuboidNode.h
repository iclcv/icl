// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/geom2/GeometryNode.h>
#include <memory>

namespace icl::geom2 {

  /// Parametric cuboid (or cube) — params are source of truth
  class ICLGeom2_API CuboidNode : public GeometryNode {
  public:
    CuboidNode(float cx, float cy, float cz, float dx, float dy, float dz);
    CuboidNode(float cx, float cy, float cz, float size);  // cube
    ~CuboidNode() override;
    CuboidNode(const CuboidNode &);
    CuboidNode &operator=(const CuboidNode &);
    CuboidNode(CuboidNode &&) noexcept;
    CuboidNode &operator=(CuboidNode &&) noexcept;
    SceneNode *deepCopy() const override;

    Vec getCenter() const;
    Vec getExtents() const;

    void setCenter(float cx, float cy, float cz);
    void setExtents(float dx, float dy, float dz);

    static std::shared_ptr<CuboidNode> create(float cx, float cy, float cz,
                                               float dx, float dy, float dz);
    static std::shared_ptr<CuboidNode> createCube(float cx, float cy, float cz, float size);

  private:
    float m_cx, m_cy, m_cz, m_dx, m_dy, m_dz;
    void generateMesh();
  };

} // namespace icl::geom2
