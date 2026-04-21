// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/LightNode.h>
#include <cmath>

namespace icl::geom2 {

  struct LightNode::Data {
    Type type = Point;
    GeomColor color{1.0f, 0.97f, 0.92f, 1.0f};  // warm white
    float intensity = 1.0f;
    bool shadowEnabled = false;
    float spotAngle = M_PI / 4.0f;
  };

  LightNode::LightNode(Type type) : m_data(std::make_unique<Data>()) { m_data->type = type; }
  LightNode::~LightNode() = default;

  LightNode::LightNode(const LightNode &o) : SceneNode(o), m_data(std::make_unique<Data>(*o.m_data)) {}

  LightNode &LightNode::operator=(const LightNode &o) {
    if (this != &o) {
      SceneNode::operator=(o);
      *m_data = *o.m_data;
    }
    return *this;
  }

  LightNode::LightNode(LightNode &&o) noexcept = default;
  LightNode &LightNode::operator=(LightNode &&o) noexcept = default;

  SceneNode *LightNode::deepCopy() const { return new LightNode(*this); }

  LightNode::Type LightNode::getLightType() const { return m_data->type; }
  void LightNode::setLightType(Type t) { m_data->type = t; }

  void LightNode::setColor(const GeomColor &c) { m_data->color = c; }
  GeomColor LightNode::getColor() const { return m_data->color; }

  void LightNode::setIntensity(float i) { m_data->intensity = i; }
  float LightNode::getIntensity() const { return m_data->intensity; }

  void LightNode::setShadowEnabled(bool on) { m_data->shadowEnabled = on; }
  bool LightNode::getShadowEnabled() const { return m_data->shadowEnabled; }

  void LightNode::setSpotAngle(float r) { m_data->spotAngle = r; }
  float LightNode::getSpotAngle() const { return m_data->spotAngle; }

} // namespace icl::geom2
