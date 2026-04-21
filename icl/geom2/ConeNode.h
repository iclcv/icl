// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/geom2/GeometryNode.h>
#include <memory>

namespace icl::geom2 {

  /// Parametric cone — params are source of truth
  class ICLGeom2_API ConeNode : public GeometryNode {
  public:
    /// Cone with apex at (cx, cy, cz+h/2), base at (cx, cy, cz-h/2)
    ConeNode(float cx, float cy, float cz,
             float rx, float ry, float h, int steps = 30);
    ~ConeNode() override;
    ConeNode(const ConeNode &);
    ConeNode &operator=(const ConeNode &);
    ConeNode(ConeNode &&) noexcept;
    ConeNode &operator=(ConeNode &&) noexcept;
    Node *deepCopy() const override;

    Vec getCenter() const;
    float getHeight() const;

    void setCenter(float cx, float cy, float cz);
    void setDimensions(float rx, float ry, float h);
    void retessellate(int steps);

    static std::shared_ptr<ConeNode> create(float cx, float cy, float cz,
                                             float rx, float ry, float h,
                                             int steps = 30);
  private:
    float m_cx, m_cy, m_cz, m_rx, m_ry, m_h;
    int m_steps;
    void generateMesh();
  };

} // namespace icl::geom2
