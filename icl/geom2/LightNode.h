// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/geom2/Node.h>
#include <icl/geom2/Primitive.h>
#include <memory>

namespace icl::geom2 {

  /// Light node — position/direction comes from the node's transform
  /** A LightNode is a leaf in the scene graph. Its world position is
      determined by its transform (inherited from Node).
      For directional lights, the transform's translation is the direction. */
  class ICLGeom2_API LightNode : public Node {
  public:
    enum Type { Point, Directional, Spot };

    LightNode(Type type = Point);
    ~LightNode() override;
    LightNode(const LightNode &);
    LightNode &operator=(const LightNode &);
    LightNode(LightNode &&) noexcept;
    LightNode &operator=(LightNode &&) noexcept;
    Node *deepCopy() const override;

    Type getLightType() const;
    void setLightType(Type type);

    void setColor(const GeomColor &color);
    GeomColor getColor() const;

    void setIntensity(float intensity);
    float getIntensity() const;

    void setShadowEnabled(bool on);
    bool getShadowEnabled() const;

    /// Spot light cone angle (radians, default pi/4)
    void setSpotAngle(float radians);
    float getSpotAngle() const;

  private:
    struct Data;
    std::unique_ptr<Data> m_data;
  };

} // namespace icl::geom2
