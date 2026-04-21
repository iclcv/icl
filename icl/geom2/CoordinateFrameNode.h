// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/geom2/GroupNode.h>
#include <memory>

namespace icl::geom2 {

  class CuboidNode;
  class ConeNode;
  class TextNode;

  /// Visible coordinate frame: three colored axes (R=X, G=Y, B=Z)
  /** In simple mode (default): three cuboid bars.
      In complex mode: cylinder-like bars + cone arrowheads + "X"/"Y"/"Z" labels. */
  class ICLGeom2_API CoordinateFrameNode : public GroupNode {
  public:
    CoordinateFrameNode(float axisLength = 100, float axisThickness = 5,
                        bool complex = false);
    Node *deepCopy() const override;

    void setParams(float axisLength, float axisThickness);
    float getAxisLength() const { return m_length; }
    float getAxisThickness() const { return m_thickness; }

    void setComplex(bool complex);
    bool isComplex() const { return m_complex; }

    static std::shared_ptr<CoordinateFrameNode> create(float axisLength = 100,
                                                       float axisThickness = 5,
                                                       bool complex = false);

  private:
    float m_length, m_thickness;
    bool m_complex;
    std::shared_ptr<CuboidNode> m_axis[3];
    std::shared_ptr<ConeNode> m_cone[3];
    std::shared_ptr<TextNode> m_label[3];
    void rebuild();
  };

} // namespace icl::geom2
