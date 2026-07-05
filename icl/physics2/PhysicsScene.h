// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/physics2/PhysicsWorld.h>
#include <icl/physics2/VehicleDriver.h>
#include <icl/physics2/SoftBodyDriver.h>
#include <icl/physics2/PaperDriver.h>
#include <icl/physics2/SensorDriver.h>
#include <icl/utils/Size.h>
#include <icl/physics2/Units.h>
#include <icl/viz3d/DefaultScene.h>
#include <memory>

#ifndef ICLPhysics2_API
#define ICLPhysics2_API
#endif

namespace icl::cv3d { class Camera; }
namespace icl::viz3d { class Node; class LightNode; class MeshNode; class Scene2MouseHandler; }
namespace icl::qt { class GLCallback; }

namespace icl::physics2 {

  /// Convenience coordinator: owns a viz3d::Scene2 + a PhysicsWorld and joins
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
    /// `mode` selects the soft-body pipeline (default = Deformable, the stable
    /// one; SoftRigid is the legacy world kept for comparison).
    explicit PhysicsScene(SoftBodyMode mode = SoftBodyMode::Deformable);
    ~PhysicsScene();

    /// Add a node to the scene AND give it a rigid body (shape derived from
    /// geometry). mass 0 = static. Returns the driver for further tuning.
    RigidBodyDriver *add(std::shared_ptr<viz3d::Node> node, float mass);

    /// Add a soft cloth patch spanned by four corners (ICL units), resX*resY
    /// nodes, fixedCornerMask pinning corners (1=c00,2=c10,4=c01,8=c11). Creates
    /// the MeshNode + SoftBodyDriver and adds both. Returns the driver.
    SoftBodyDriver *addCloth(const Vec &c00, const Vec &c10,
                             const Vec &c01, const Vec &c11,
                             int resX, int resY, int fixedCornerMask, float totalMass);

    /// Add a fold-aware paper sheet (the PhysicsPaper3 transplant). \a cells =
    /// grid resolution, \a corners (4, ul/ur/ll/lr, ICL units; null = default
    /// sheet). Creates the MeshNode + PaperDriver and adds both. Requires a
    /// SoftRigid world. Returns the driver (attach FoldDriver / PaperMoverDriver
    /// to its node() for interaction). Behaviour drivers compose on one node.
    PaperDriver *addPaper(const utils::Size &cells, const Vec *corners = nullptr,
                          bool enableSelfCollision = false,
                          float initialStiffness = -1.f, float maxLinkDist = 0.4f);

    /// Add a node as a trigger zone (ghost). Detects overlapping bodies without
    /// any collision response. Set the node invisible for an invisible trigger.
    SensorDriver *addSensor(std::shared_ptr<viz3d::Node> node);

    /// Turn \a chassis into a raycast vehicle and add it (chassis + its four
    /// wheel nodes) to the scene. Returns the driver for control + the chase cam.
    VehicleDriver *addVehicle(std::shared_ptr<viz3d::Node> chassis,
                              const VehicleDriver::Config &cfg = {});

    // --- constraints (joints), forwarded to the world (both nodes must already
    //     have rigid bodies, e.g. via add()) ---
    /// Rotation free about \a axis only; all translation locked (e.g. a door).
    std::shared_ptr<Constraint> addHinge(viz3d::NodePtr a, viz3d::NodePtr b,
                                         const Vec &pivA, const Vec &pivB, int axis);
    /// Translation free along \a axis only; all rotation locked.
    std::shared_ptr<Constraint> addSlider(viz3d::NodePtr a, viz3d::NodePtr b,
                                          const Vec &pivA, const Vec &pivB, int axis);
    /// All rotation free; all translation locked (a ball-and-socket joint).
    std::shared_ptr<Constraint> addBallSocket(viz3d::NodePtr a, viz3d::NodePtr b,
                                              const Vec &pivA, const Vec &pivB);
    /// Fully configurable: all 6 axes locked by default, open with the setters.
    std::shared_ptr<Constraint> addSixDOF(viz3d::NodePtr a, viz3d::NodePtr b,
                                          const Vec &pivA, const Vec &pivB);
    /// Spring-bind \a obj's \a localOffset point to a world-space \a worldPoint.
    std::shared_ptr<SpringConstraint> addSpring(viz3d::NodePtr obj, const Vec &localOffset,
                                                const Vec &worldPoint, float stiffness, float damping);

    /// Furnish the scene with a default environment (camera, lamp rig, ground)
    /// plus a matching static ground collider, ready to add() bodies onto.
    /** Z-up (physics convention, matching the default gravity). The visual
        ground is DefaultScene's checkerboard; the collider is an invisible box
        whose top is coincident with it, so bodies rest exactly on what's drawn.
        \a extent is the characteristic scene size (mm) — drives ground/lights/
        camera and the ground level (top at -extent/2 - 2%). */
    void setupDefault(viz3d::DefaultScene::SceneType type =
                          viz3d::DefaultScene::SceneType::Studio,
                      float extent = 1000.f);

    /// Add a non-physical node (decoration, coordinate frame, ...).
    void addNode(std::shared_ptr<viz3d::Node> node);
    void addLight(std::shared_ptr<viz3d::LightNode> light);
    void addCamera(const cv3d::Camera &cam);
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
    viz3d::Scene2MouseHandler *getMouseHandler(int cameraIndex);

    // --- escape hatches ---
    viz3d::Scene2 &scene();
    PhysicsWorld &world();

  private:
    PhysicsWorld m_world;        // declared first  -> destroyed last
    viz3d::DefaultScene m_scene; // declared second -> destroyed first (is-a Scene2)
    std::shared_ptr<viz3d::MeshNode> m_debugNode;
    bool m_debugEnabled = false;
  };

} // namespace icl::physics2
