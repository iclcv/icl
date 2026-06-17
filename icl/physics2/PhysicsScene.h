// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/physics2/PhysicsWorld.h>
#include <icl/physics2/SoftBodyDriver.h>
#include <icl/physics2/SensorDriver.h>
#include <icl/physics2/Units.h>
#include <icl/geom2/Scene2.h>
#include <memory>

#ifndef ICLPhysics2_API
#define ICLPhysics2_API
#endif

namespace icl::geom { class Camera; }
namespace icl::geom2 { class Node; class LightNode; class MeshNode; class Scene2MouseHandler; }
namespace icl::qt { class GLCallback; }

namespace icl::physics2 {

  /// Convenience coordinator: owns a geom2::Scene2 + a PhysicsWorld and joins
  /// them with a one-call add().
  /** Composition, NOT inheritance — the world (sim thread) and the scene (UI
      thread) stay separate concerns. The facade just sequences them. For the
      advanced multi-world case, drop to scene() / world() and wire drivers
      directly.

      Frame loop (single-world app):
        scene.start();                 // physics runs on its own thread
        ... per render frame:
          scene.sync(dt);              // pull body poses into nodes (UI thread)
          canvas.render();

      Member order matters: the world is declared first so it is destroyed last
      — the scene (and its nodes) tear down while the world is still alive, so
      driver onDetach() can safely remove its body. */
  class ICLPhysics2_API PhysicsScene {
  public:
    PhysicsScene();
    ~PhysicsScene();

    /// Add a node to the scene AND give it a rigid body (shape derived from
    /// geometry). mass 0 = static. Returns the driver for further tuning.
    RigidBodyDriver *add(std::shared_ptr<geom2::Node> node, float mass);

    /// Add a soft cloth patch spanned by four corners (ICL units), resX*resY
    /// nodes, fixedCornerMask pinning corners (1=c00,2=c10,4=c01,8=c11). Creates
    /// the MeshNode + SoftBodyDriver and adds both. Returns the driver.
    SoftBodyDriver *addCloth(const Vec &c00, const Vec &c10,
                             const Vec &c01, const Vec &c11,
                             int resX, int resY, int fixedCornerMask, float totalMass);

    /// Add a node as a trigger zone (ghost). Detects overlapping bodies without
    /// any collision response. Set the node invisible for an invisible trigger.
    SensorDriver *addSensor(std::shared_ptr<geom2::Node> node);

    /// Add a non-physical node (decoration, coordinate frame, ...).
    void addNode(std::shared_ptr<geom2::Node> node);
    void addLight(std::shared_ptr<geom2::LightNode> light);
    void addCamera(const geom::Camera &cam);
    void setBounds(float maxDim);

    // --- simulation control (forwarded to the world) ---
    void start(float hz = 120.f);
    void stop();
    void stepOnce(float dt, int maxSubSteps = 10, float fixedTimeStep = 1.f/120.f);

    // --- render-side pull (UI thread) ---
    void sync(double dt, double alpha = 1.0);

    /// Overlay the Bullet collision wireframe (rebuilt each sync). The
    /// diagnostic that makes collision-vs-render divergence visible.
    void setDebugDrawEnabled(bool on);
    bool getDebugDrawEnabled() const;

    // --- Qt integration ---
    std::shared_ptr<qt::GLCallback> getGLCallback(int cameraIndex);
    geom2::Scene2MouseHandler *getMouseHandler(int cameraIndex);

    // --- escape hatches ---
    geom2::Scene2 &scene();
    PhysicsWorld &world();

  private:
    PhysicsWorld m_world;     // declared first  -> destroyed last
    geom2::Scene2 m_scene;    // declared second -> destroyed first
    std::shared_ptr<geom2::MeshNode> m_debugNode;
    bool m_debugEnabled = false;
  };

} // namespace icl::physics2
