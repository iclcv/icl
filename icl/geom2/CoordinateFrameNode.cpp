// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/CoordinateFrameNode.h>
#include <icl/geom2/CuboidNode.h>
#include <icl/geom2/ConeNode.h>
#include <icl/geom2/TextNode.h>
#include <icl/core/Img.h>
#include <icl/geom/Material.h>

namespace icl::geom2 {

  static const GeomColor AXIS_COLORS[3] = {
    {255, 0, 0, 255},
    {0, 255, 0, 255},
    {0, 0, 255, 255}
  };
  static const char *AXIS_NAMES[3] = {"X", "Y", "Z"};

  CoordinateFrameNode::CoordinateFrameNode(float axisLength, float axisThickness,
                                           bool complex)
      : m_length(axisLength), m_thickness(axisThickness), m_complex(complex) {
    for (int i = 0; i < 3; i++) {
      m_axis[i] = std::make_shared<CuboidNode>(0, 0, 0, 1, 1, 1);
      m_axis[i]->setMaterial(geom::Material::fromColor(AXIS_COLORS[i]));
      m_axis[i]->setPrimitiveVisible(PrimLine | PrimVertex, false);
      addChild(m_axis[i]);

      m_cone[i] = std::make_shared<ConeNode>(0, 0, 0, 1, 1, 1, 12);
      m_cone[i]->setMaterial(geom::Material::fromColor(AXIS_COLORS[i]));
      m_cone[i]->setPrimitiveVisible(PrimLine | PrimVertex, false);
      addChild(m_cone[i]);

      m_label[i] = TextNode::create(AXIS_NAMES[i], axisThickness * 3, AXIS_COLORS[i]);
      addChild(m_label[i]);
    }
    rebuild();
  }

  Node *CoordinateFrameNode::deepCopy() const {
    return new CoordinateFrameNode(m_length, m_thickness, m_complex);
  }

  void CoordinateFrameNode::setParams(float axisLength, float axisThickness) {
    m_length = axisLength;
    m_thickness = axisThickness;
    rebuild();
  }

  void CoordinateFrameNode::setComplex(bool complex) {
    if (m_complex != complex) {
      m_complex = complex;
      rebuild();
    }
  }

  std::shared_ptr<CoordinateFrameNode> CoordinateFrameNode::create(
      float axisLength, float axisThickness, bool complex) {
    return std::make_shared<CoordinateFrameNode>(axisLength, axisThickness, complex);
  }

  void CoordinateFrameNode::rebuild() {
    float l = m_length, t = m_thickness;
    float coneH = m_complex ? t * 4 : 0;
    float coneR = m_complex ? t * 3.0f : 0;
    float barLen = m_complex ? l - coneH : l;


    m_axis[0]->setCenter(barLen / 2, 0, 0);
    m_axis[0]->setExtents(barLen, t, t);
    m_axis[1]->setCenter(0, barLen / 2, 0);
    m_axis[1]->setExtents(t, barLen, t);
    m_axis[2]->setCenter(0, 0, barLen / 2);
    m_axis[2]->setExtents(t, t, barLen);

    if (!m_complex) {
      for(int i = 0; i < 3; i++) {
        m_cone[i]->setVisible(false);
        m_label[i]->setVisible(false);
      }
    } else {
      // Same approach as old ComplexCoordinateFrameSceneObject:
      // bake offset into geometry (cone center at (0,0,l)), then rotate around origin
      float conePos = barLen + coneH / 2;
      float rxs[3] = {0, -float(M_PI_2), 0};
      float rys[3] = {float(M_PI_2), 0, 0};
      for (int i = 0; i < 3; i++) {
        m_cone[i]->setCenter(0, 0, conePos);  // offset along Z
        m_cone[i]->setDimensions(coneR, coneR, coneH);
        m_cone[i]->removeTransformation();
        m_cone[i]->rotate(rxs[i], rys[i], 0);  // swing into correct axis
        m_cone[i]->setVisible(true);
      }

      float labelOffset = l + coneH * 0.5f;
      for(int i = 0; i < 3; i++) {
        m_label[i]->removeTransformation();
        float lx = (i == 0) ? labelOffset : 0;
        float ly = (i == 1) ? labelOffset : 0;
        float lz = (i == 2) ? labelOffset : 0;
        m_label[i]->translate(lx, ly, lz);
        m_label[i]->setBillboardHeight(t * 3);
        m_label[i]->setVisible(true);
      }
    }
  }

} // namespace icl::geom2
