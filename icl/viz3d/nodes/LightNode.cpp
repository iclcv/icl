// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/viz3d/nodes/LightNode.h>
#include <cmath>

namespace icl::viz3d {

  struct LightNode::Data {
    Type type = Point;
    GeomColor color{1.0f, 0.97f, 0.92f, 1.0f};  // warm white
    float intensity = 1.0f;
    bool shadowEnabled = false;
    float softShadowRadius = 0.0f;
    float spotAngle = M_PI / 4.0f;
  };

  LightNode::LightNode(Type type) : m_data(std::make_unique<Data>()) { m_data->type = type; }
  LightNode::~LightNode() = default;

  std::shared_ptr<LightNode> LightNode::point(float x, float y, float z,
                                              const GeomColor &color, float intensity,
                                              bool shadows) {
    auto l = std::make_shared<LightNode>(Point);
    l->setColor(color);
    l->setIntensity(intensity);
    l->translate(x, y, z);
    l->setShadowEnabled(shadows);
    return l;
  }

  std::shared_ptr<LightNode> LightNode::directional(float dx, float dy, float dz,
                                                    const GeomColor &color, float intensity) {
    auto l = std::make_shared<LightNode>(Directional);
    l->setColor(color);
    l->setIntensity(intensity);
    l->translate(dx, dy, dz);   // for directional lights the translation is the direction
    return l;
  }

  LightNode::LightNode(const LightNode &o) : Node(o), m_data(std::make_unique<Data>(*o.m_data)) {}

  LightNode &LightNode::operator=(const LightNode &o) {
    if (this != &o) {
      Node::operator=(o);
      *m_data = *o.m_data;
    }
    return *this;
  }

  LightNode::LightNode(LightNode &&o) noexcept = default;
  LightNode &LightNode::operator=(LightNode &&o) noexcept = default;

  NodePtr LightNode::deepCopy() const { return std::make_shared<LightNode>(*this); }

  LightNode::Type LightNode::getLightType() const { return m_data->type; }
  void LightNode::setLightType(Type t) { m_data->type = t; }

  void LightNode::setColor(const GeomColor &c) { m_data->color = c; }
  GeomColor LightNode::getColor() const { return m_data->color; }

  void LightNode::setIntensity(float i) { m_data->intensity = i; }
  float LightNode::getIntensity() const { return m_data->intensity; }

  void LightNode::setShadowEnabled(bool on) { m_data->shadowEnabled = on; }
  bool LightNode::getShadowEnabled() const { return m_data->shadowEnabled; }

  void LightNode::setSoftShadowRadius(float r) { m_data->softShadowRadius = r; }
  float LightNode::getSoftShadowRadius() const { return m_data->softShadowRadius; }

  void LightNode::setSpotAngle(float r) { m_data->spotAngle = r; }
  float LightNode::getSpotAngle() const { return m_data->spotAngle; }

} // namespace icl::viz3d
