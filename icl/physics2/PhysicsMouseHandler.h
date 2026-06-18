// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/qt/MouseHandler.h>
#include <memory>

#ifndef ICLPhysics2_API
#define ICLPhysics2_API
#endif

namespace icl::geom2 { class Scene2; }

namespace icl::physics2 {

  class PhysicsWorld;

  /// Shift+Left-drag object grabbing for physics2 scenes (one chain link).
  /** A plain qt::MouseHandler implementing the unified picking path: a
      Shift+Left press raycasts via Scene2::findObject, resolves the hit node's
      RigidBodyDriver, and grabs its dynamic body with a point-to-point spring;
      dragging moves the grab target; release lets go. All Bullet mutations go
      through the world command queue (sim-thread safe). Returns Processed while
      grabbing, Forward otherwise so the camera handler installed after this one
      drives navigation. */
  class ICLPhysics2_API PhysicsMouseHandler : public qt::MouseHandler {
  public:
    PhysicsMouseHandler(int cameraIndex, geom2::Scene2 *scene, PhysicsWorld *world);
    ~PhysicsMouseHandler() override;

    qt::MouseResult process(const qt::MouseEvent &e) override;

  private:
    void releaseGrab();

    struct Data;
    std::unique_ptr<Data> m_data;
  };

} // namespace icl::physics2
