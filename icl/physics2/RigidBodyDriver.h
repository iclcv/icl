// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/geom2/Driver.h>
#include <icl/physics2/Units.h>
#include <memory>

#ifndef ICLPhysics2_API
#define ICLPhysics2_API
#endif

class btRigidBody;

namespace icl::physics2 {

  class PhysicsWorld;

  /// A geom2 Driver that binds a Bullet rigid body to its host node.
  /** This is the physics2 replacement for `RigidObject` + `geom::SceneObject`
      inheritance. The node owns the driver; the driver owns the Bullet body,
      a collision shape (derived from the node geometry by default), and a
      MotionState that publishes the body's pose into a `StateSlot` each
      simulation step.

      Thread split (documented contract):
        - sim thread : the MotionState writes the slot during PhysicsWorld::step;
        - UI thread  : sync() samples the slot (optionally interpolated) and
                       writes the node transform; the setters below mutate the
                       body under the world lock.

      The body is registered with the world on onAttach() and removed on
      onDetach() (called from removeDriver() or the node's destructor). */
  class ICLPhysics2_API RigidBodyDriver : public geom2::Driver {
  public:
    /// Born from PhysicsWorld::addRigidBody(node, mass) — collision shape is
    /// derived from the node geometry. mass 0 = static body.
    RigidBodyDriver(PhysicsWorld &world, float mass);
    ~RigidBodyDriver() override;

    void onAttach() override;
    void onDetach() override;
    void sync(double dt, double alpha) override;

    /// Current world pose in ICL units (samples the slot; valid headless).
    Mat getPose() const;

    btRigidBody *body() const;

    // --- kinematic / user-controlled body (the tilt-maze case) ---
    /// Flag the body kinematic (mass 0, never sleeps): its pose is driven by
    /// the user each frame, collisions push dynamic bodies correctly.
    void setKinematic(bool on);

    /// Set the kinematic pose (ICL units). Routed through the world command
    /// queue so the sim thread updates the MotionState (what Bullet reads) +
    /// the render slot together — collision and render stay consistent.
    void setKinematicTransform(const Mat &m);

    // --- tunables (no-ops before the body exists) ---
    void setMass(float mass);
    void setFriction(float f);
    void setRestitution(float r);
    void setRollingFriction(float rf);
    void setLinearVelocity(const Vec &v);
    void setAngularVelocity(const Vec &v);
    void applyCentralForce(const Vec &f);

    /// Continuous collision detection: catch fast/small bodies that would
    /// tunnel through thin geometry in one step. Both args in ICL units;
    /// motionThreshold ~ body size, sweptRadius ~ a fraction of it.
    void setCcd(float motionThreshold, float sweptSphereRadius);

    /// Collision filter group + mask (re-adds the body with the new filter).
    void setCollisionFilter(int group, int mask);

  private:
    struct Data;
    std::unique_ptr<Data> m_data;
  };

} // namespace icl::physics2
