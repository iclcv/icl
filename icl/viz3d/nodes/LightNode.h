// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/viz3d/nodes/Node.h>
#include <icl/viz3d/render/Primitive.h>
#include <memory>

namespace icl::viz3d {

  /// Light node — position/direction comes from the node's transform
  /** A LightNode is a leaf in the scene graph. Its world position is
      determined by its transform (inherited from Node).
      For directional lights, the transform's translation is the direction. */
  class ICLViz3d_API LightNode : public Node {
  public:
    enum Type { Point, Directional, Spot };

    LightNode(Type type = Point);
    ~LightNode() override;

    /// Convenience factory: a Point light at (x,y,z). NB the colour is 0..255
    /// (LightNode / Cycles convention), NOT 0..1. Collapses the usual
    /// make_shared + setColor + setIntensity + translate + setShadowEnabled dance.
    static std::shared_ptr<LightNode> point(float x, float y, float z,
                                            const GeomColor &color = GeomColor(255, 247, 235, 255),
                                            float intensity = 1.0f,
                                            bool shadows = true);

    /// Convenience factory: a Directional light shining along (dx,dy,dz).
    static std::shared_ptr<LightNode> directional(float dx, float dy, float dz,
                                                  const GeomColor &color = GeomColor(255, 247, 235, 255),
                                                  float intensity = 1.0f);

    LightNode(const LightNode &);
    LightNode &operator=(const LightNode &);
    LightNode(LightNode &&) noexcept;
    LightNode &operator=(LightNode &&) noexcept;
    NodePtr deepCopy() const override;

    Type getLightType() const;
    void setLightType(Type type);

    void setColor(const GeomColor &color);
    GeomColor getColor() const;

    void setIntensity(float intensity);
    float getIntensity() const;

    void setShadowEnabled(bool on);
    bool getShadowEnabled() const;

    /// Soft shadow PCF radius in texels (0 = hard shadow, default)
    /** Higher values produce softer shadow edges. Typical range: 1-5.
        Uses a 16-sample Poisson disk filter. */
    void setSoftShadowRadius(float texels);
    float getSoftShadowRadius() const;

    /// Spot light cone angle (radians, default pi/4)
    void setSpotAngle(float radians);
    float getSpotAngle() const;

  private:
    struct Data;
    std::unique_ptr<Data> m_data;
  };

} // namespace icl::viz3d
