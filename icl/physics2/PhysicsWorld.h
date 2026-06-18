// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/physics2/Units.h>
#include <icl/physics2/RigidBodyDriver.h>
#include <icl/geom2/Node.h>
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
struct btSoftBodyWorldInfo;

namespace icl::geom2 { class Driver; }

namespace icl::physics2 {

  /// One collision-shape wireframe segment in ICL units (for debug draw).
  struct DebugLine { Vec a, b; };

  /// Which Bullet soft-body pipeline the world runs.
  /** `Deformable` (default) builds a `btDeformableMultiBodyDynamicsWorld`: soft
      bodies are driven by the deformable solver whose contact *projection*
      (split-impulse + ERP) is stable at rest. `SoftRigid` builds the legacy
      `btSoftRigidDynamicsWorld` whose impulse contact solver pumps energy at rest
      (cloth explodes — band-aided, not fixed). The same mass-spring cloth is
      stable in the deformable world and unstable in the legacy one, so
      `Deformable` is the default. Chosen at construction (it picks the world
      type). */
  enum class SoftBodyMode { Deformable, SoftRigid };

  /// A Bullet dynamics world that simulates on its own thread, decoupled from
  /// rendering.
  /** physics2's world is deliberately thin: it owns the Bullet world + the
      unit/scale policy, runs a fixed-timestep loop (so physical accuracy is
      independent of render framerate), and serves as the factory for drivers.
      It never touches a geom2 node — bodies publish their pose into per-driver
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
        std::function<void(geom2::Driver *a, geom2::Driver *b, const Vec &worldPoint)>;
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
    D *addDriver(std::shared_ptr<geom2::Node> node, A &&... args) {
      auto d = node->addDriver<D>(*this, std::forward<A>(args)...);
      return d.get();
    }
    /// Convenience: rigid body with a collision shape derived from the node.
    RigidBodyDriver *addRigidBody(std::shared_ptr<geom2::Node> node, float mass) {
      return addDriver<RigidBodyDriver>(std::move(node), mass);
    }

    // --- escape hatch + BasicLockable (so std::scoped_lock<PhysicsWorld> works) ---
    btDynamicsWorld *getDynamicsWorld();
    void lock();
    void unlock();

  private:
    struct Data;
    std::unique_ptr<Data> m_data;
  };

} // namespace icl::physics2
