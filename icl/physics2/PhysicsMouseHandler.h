// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <memory>

#ifndef ICLPhysics2_API
#define ICLPhysics2_API
#endif

namespace icl::geom2 { class Scene2; }

namespace icl::physics2 {

  class PhysicsWorld;

  /// Camera navigation + Shift+Left-drag object grabbing for physics2 scenes.
  /** Extends geom2::Scene2MouseHandler (camera nav) with the unified picking
      path: a Shift+Left press raycasts via Scene2::findObject, resolves the hit
      node's RigidBodyDriver, and grabs its dynamic body with a point-to-point
      spring; dragging moves the grab target; release lets go. All Bullet
      mutations go through the world command queue (sim-thread safe). */
  class ICLPhysics2_API PhysicsMouseHandler : public geom2::Scene2MouseHandler {
  public:
    PhysicsMouseHandler(int cameraIndex, geom2::Scene2 *scene, PhysicsWorld *world);
    ~PhysicsMouseHandler() override;

    void process(const qt::MouseEvent &e) override;

  private:
    void releaseGrab();

    struct Data;
    std::unique_ptr<Data> m_data;
  };

} // namespace icl::physics2
