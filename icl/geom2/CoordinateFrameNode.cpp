// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/CoordinateFrameNode.h>
#include <icl/core/Img.h>
#include <icl/geom/Material.h>

namespace icl::geom2 {

  CoordinateFrameNode::CoordinateFrameNode(float axisLength, float axisThickness)
      : m_length(axisLength), m_thickness(axisThickness) {
    // Create three cuboid children: X=red, Y=green, Z=blue
    GeomColor colors[3] = {
      {255, 0, 0, 255},
      {0, 255, 0, 255},
      {0, 0, 255, 255}
    };
    for (int i = 0; i < 3; i++) {
      m_axis[i] = std::make_shared<CuboidNode>(0, 0, 0, 1, 1, 1);
      m_axis[i]->setMaterial(geom::Material::fromColor(colors[i]));
      m_axis[i]->setPrimitiveVisible(PrimLine | PrimVertex, false);
      addChild(m_axis[i]);
    }
    rebuild();
  }

  Node *CoordinateFrameNode::deepCopy() const {
    return new CoordinateFrameNode(m_length, m_thickness);
  }

  void CoordinateFrameNode::setParams(float axisLength, float axisThickness) {
    m_length = axisLength;
    m_thickness = axisThickness;
    rebuild();
  }

  std::shared_ptr<CoordinateFrameNode> CoordinateFrameNode::create(float axisLength,
                                                                      float axisThickness) {
    return std::make_shared<CoordinateFrameNode>(axisLength, axisThickness);
  }

  void CoordinateFrameNode::rebuild() {
    float l = m_length, t = m_thickness;
    for (int i = 0; i < 3; i++) {
      float dx = (i == 0) ? l : t;
      float dy = (i == 1) ? l : t;
      float dz = (i == 2) ? l : t;
      float cx = (i == 0) ? l / 2 : 0;
      float cy = (i == 1) ? l / 2 : 0;
      float cz = (i == 2) ? l / 2 : 0;
      m_axis[i]->setCenter(cx, cy, cz);
      m_axis[i]->setExtents(dx, dy, dz);
    }
  }

} // namespace icl::geom2
