// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/geom2/GroupNode.h>
#include <icl/geom2/CuboidNode.h>
#include <memory>

namespace icl::geom2 {

  /// Visible coordinate frame: three colored cuboid axes (R=X, G=Y, B=Z)
  class ICLGeom2_API CoordinateFrameNode : public GroupNode {
  public:
    CoordinateFrameNode(float axisLength = 100, float axisThickness = 5);
    Node *deepCopy() const override;

    void setParams(float axisLength, float axisThickness);
    float getAxisLength() const { return m_length; }
    float getAxisThickness() const { return m_thickness; }

    static std::shared_ptr<CoordinateFrameNode> create(float axisLength = 100,
                                                         float axisThickness = 5);

  private:
    float m_length, m_thickness;
    std::shared_ptr<CuboidNode> m_axis[3];
    void rebuild();
  };

} // namespace icl::geom2
