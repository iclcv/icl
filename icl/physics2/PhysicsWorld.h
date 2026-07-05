// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/physics2/Units.h>
#include <icl/physics2/RigidBodyDriver.h>
#include <icl/physics2/Constraint.h>
#include <icl/viz3d/Node.h>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#ifndef ICLPhysics2_API
#define ICLPhysics2_API
#endif

class btRigidBody;
class btSoftBody;
class btCollisionObject;
class btDynamicsWorld;
class btActionInterface;
struct btSoftBodyWorldInfo;

namespace icl::viz3d { class Driver; }

namespace icl::physics2 {

  /// One collision-shape wireframe segment in ICL units (for debug draw).
  struct DebugLine { Vec a, b; };

  /// Which Bullet world the simulation runs in.
  /** `Deformable` (default) is the **unified world** — a
      `btDeformableMultiBodyDynamicsWorld` that hosts rigid bodies, 6DOF joints
      (`Constraint`) *and* deformable cloth in one solver, with contact
      *projection* (split-impulse + ERP) that is stable at rest. This is the
      everything-world; prefer it. `SoftRigid` builds the legacy
      `btSoftRigidDynamicsWorld` whose impulse contact solver pumps energy into
      mass-spring cloth at rest (band-aided, not fixed) — kept only for the
      cluster self-collision the fold-aware paper (`PaperDriver`) needs. Chosen
      at construction (it picks the world type). */
  enum class SoftBodyMode { Deformable, SoftRigid };

  /// A Bullet dynamics world that simulates on its own thread, decoupled from
  /// rendering.
  /** physics2's world is deliberately thin: it owns the Bullet world + the
      unit/scale policy, runs a fixed-timestep loop (so physical accuracy is
      independent of render framerate), and serves as the factory for drivers.
      It never touches a viz3d node — bodies publish their pose into per-driver
      `StateSlot`s, which the render thread samples via `Scene2::sync`.

      Threading: `start(hz)` spawns a fixed-step loop; `stepOnce()` advances one
      step synchronously (deterministic — used by tests / headless). Structural
      mutations (`addBody`/`removeBody`) and `stepOnce` share one recursive
      mutex, so a UI-thread add blocks at most one in-flight step. (The
      UI->sim *command queue* for kinematic targets / mouse grab lands in
      Phase 2.) */
  class ICLPhysics2_API PhysicsWorld {
  public:
    PhysicsWorld(const PhysicsWorld &) = delete;
    PhysicsWorld &operator=(const PhysicsWorld &) = delete;

    explicit PhysicsWorld(SoftBodyMode mode = SoftBodyMode::Deformable);
    ~PhysicsWorld();

    /// True if this world runs the deformable soft-body pipeline (the default).
    bool isDeformable() const;

    // --- units / scale ---
    Units getUnits() const;
    void setUnitScale(float iclToBullet);

    // --- gravity (ICL units, mm/s^2; default (0,0,-9810)) ---
    void setGravity(const Vec &g);
    void setGravityEnabled(bool on);

    /// Contact event: invoked per contacting pair each step (sim thread). The
    /// driver pointers are the bodies' owners (cast to RigidBodyDriver* etc.).
    using ContactCallback =
        std::function<void(viz3d::Driver *a, viz3d::Driver *b, const Vec &worldPoint)>;
    /// Per-body force in ICL units as a function of the body's world position.
    using ForceField = std::function<Vec(const Vec &posIcl)>;

    // --- body registration (lock-serialized against the sim loop) ---
    void addBody(btRigidBody *body);
    void removeBody(btRigidBody *body);
    /// Re-add a body with a new collision filter group/mask.
    void setBodyFilter(btRigidBody *body, int group, int mask);

    // --- contact events + force fields ---
    void setContactCallback(ContactCallback cb);
    int addForceField(ForceField field);   ///< returns a handle for removal
    void removeForceField(int handle);

    // --- soft bodies ---
    void addSoftBody(btSoftBody *body);
    void removeSoftBody(btSoftBody *body);
    btSoftBodyWorldInfo *getSoftBodyWorldInfo();

    // --- deformable-mode cloth forces (no-op in SoftRigid mode) ---
    /// Attach the standard cloth forces (mass-spring + gravity) a soft body needs
    /// to simulate in the deformable world. The world owns the force objects and
    /// frees them on `removeSoftBody`. Must be called after `addSoftBody`.
    void addClothForces(btSoftBody *body, float stiffness, float damping);
    /// Live-update a deformable cloth's spring stiffness / damping (sim thread).
    void setClothStiffness(btSoftBody *body, float stiffness, float damping);

    // --- collision objects (ghost sensors etc., no dynamics) ---
    void addCollisionObject(btCollisionObject *obj, int group, int mask);
    void removeCollisionObject(btCollisionObject *obj);

    // --- actions (btActionInterface ticked each step, e.g. a raycast vehicle) ---
    void addAction(btActionInterface *action);
    void removeAction(btActionInterface *action);

    /// Register/unregister a post-step capture hook (soft bodies snapshot their
    /// node positions into a render buffer here, on the sim thread). `token`
    /// identifies the hook for removal.
    void addCapture(void *token, std::function<void()> fn);
    void removeCapture(void *token);

    // --- UI -> sim command queue ---
    /// Enqueue a mutation to run on the sim thread, drained at the start of the
    /// next step (before stepping). Used for kinematic-target writes and mouse
    /// grab so the UI thread never mutates a body mid-step.
    void enqueue(std::function<void()> cmd);

    // --- collision-shape debug draw ---
    /// Run Bullet's debug draw and return the collision wireframe in ICL units
    /// (the diagnostic that makes collision-vs-render divergence visible).
    std::vector<DebugLine> getDebugLines();

    // --- stepping ---
    /// Advance one step synchronously (deterministic; no render thread needed).
    void stepOnce(float dt, int maxSubSteps = 10, float fixedTimeStep = 1.f/120.f);

    /// Run the fixed-timestep loop on a background thread (hz = sim rate).
    void start(float hz = 120.f);
    void stop();
    bool isRunning() const;

    // --- driver factory (the world injects itself; nothing to pass) ---
    /// Generic: world.addDriver<MyDriver>(node, args...) for custom drivers.
    template<class D, class... A>
    D *addDriver(std::shared_ptr<viz3d::Node> node, A &&... args) {
      auto d = node->addDriver<D>(*this, std::forward<A>(args)...);
      return d.get();
    }
    /// Convenience: rigid body with a collision shape derived from the node.
    RigidBodyDriver *addRigidBody(std::shared_ptr<viz3d::Node> node, float mass) {
      return addDriver<RigidBodyDriver>(std::move(node), mass);
    }

    // --- constraints (joints) — the world is the factory; it co-owns the
    //     returned handle and removes it automatically when a referenced body
    //     leaves the world. \a pivA / \a pivB are the joint pivot in each body's
    //     local space (ICL units); \a axis is 0/1/2 (x/y/z). ---
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
    /// Detach + delete a constraint (the handle goes inert).
    void removeConstraint(const std::shared_ptr<Constraint> &c);
    /// Number of live (active) constraints in the world.
    int getConstraintCount() const;

    // --- escape hatch + BasicLockable (so std::scoped_lock<PhysicsWorld> works) ---
    btDynamicsWorld *getDynamicsWorld();
    void lock();
    void unlock();

  private:
    /// Take co-ownership of a factory-built constraint (called by the factories).
    void registerConstraint(std::shared_ptr<Constraint> c);

    /// Shared builder for the four 6DOF presets (axis used by Hinge/Slider only).
    enum class Joint { SixDOF, Hinge, Slider, BallSocket };
    std::shared_ptr<Constraint> makeDof(viz3d::NodePtr a, viz3d::NodePtr b,
                                        const Vec &pivA, const Vec &pivB,
                                        Joint kind, int axis);

    struct Data;
    std::unique_ptr<Data> m_data;
  };

} // namespace icl::physics2
