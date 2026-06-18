// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/qt/MouseHandler.h>
#include <icl/geom/GeomDefs.h>

namespace icl::geom2 { class Scene2; }

namespace icl::physics {

  class PhysicsWorld;
  class RigidObject;
  class Object2PointConstraint;

  /// geom2 object-grabbing handler (one link in the handler chain).
  /** A plain qt::MouseHandler implementing the legacy "grab" behaviour:
      Shift+Left-drag ray-casts into the PhysicsWorld and drags the hit
      RigidObject via a temporary Object2PointConstraint. Returns Processed while
      grabbing, Forward otherwise so the camera handler installed after this one
      drives navigation. */
  class ICLPhysics_API PhysicsMouseHandler2 : public qt::MouseHandler {
    public:
    PhysicsMouseHandler2(int cameraIndex, geom2::Scene2 *scene, PhysicsWorld *world);
    ~PhysicsMouseHandler2();

    qt::MouseResult process(const qt::MouseEvent &e) override;

    private:
    void removeConstraint();

    geom2::Scene2 *m_scene;
    PhysicsWorld *m_world;
    int m_camIndex;
    RigidObject *m_selected;
    Object2PointConstraint *m_constraint;
    geom::Vec m_hitPoint;
  };

} // namespace icl::physics
