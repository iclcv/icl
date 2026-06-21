// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/physics2/Units.h>
#include <memory>

#ifndef ICLPhysics2_API
#define ICLPhysics2_API
#endif

namespace icl::physics2 {

  class PhysicsWorld;

  /// A two-body joint — the world-owned handle to a Bullet constraint.
  /** The legacy `SixDOF/Slider/Hinge/BallSocket` class tree is really *one*
      Bullet primitive (`btGeneric6DofConstraint`) with different limit presets,
      so physics2 exposes a single `Constraint` and lets the world's factories
      (`PhysicsWorld::addHinge` / `addSlider` / `addBallSocket` / `addSixDOF`)
      pick the preset. `SpringConstraint` is the one structural outlier.

      Lifetime — a constraint is a **cross-edge**: it references two bodies it
      must not keep alive. The *world* co-owns the handle; the constraint holds
      `weak_ptr<RigidBodyDriver>` to each body. When a referenced body leaves the
      world (its node/driver torn down), the world removes the constraint *first*
      (Bullet requires a joint to die before its bodies) and the handle goes
      **inert** — `isActive()` returns false and every setter no-ops, so a stale
      handle can never dangle. Construct only via the world factories.

      World choice: joints work in **both** world modes, including the default
      `Deformable` (unified) world — it hosts rigid bodies + 6DOF joints + cloth
      in one solver. (No `SoftRigid` requirement; that earlier claim was a
      misdiagnosis of the axis caveat below.)

      Axis caveat (`addHinge`/`addSlider`): `btGeneric6DofConstraint` limits its
      *middle* angular axis (Y, index 1) to +/-90deg via Euler angles, so a joint
      freed about Y misbehaves. Prefer axis X (0) or Z (2). */
  class ICLPhysics2_API Constraint {
  public:
    virtual ~Constraint();
    Constraint(const Constraint &) = delete;
    Constraint &operator=(const Constraint &) = delete;

    /// False once a referenced body has left the world (handle is then inert).
    bool isActive() const;

    /// Linear motion limits in the constraint frame (ICL units). Per axis:
    /// lower < upper = limited range, lower == upper = locked, lower > upper = free.
    void setLinearLimits(const Vec &lower, const Vec &upper);
    /// Angular limits (radians), same lower/upper convention as setLinearLimits.
    void setAngularLimits(const Vec &lower, const Vec &upper);
    /// Drive translation on \a axis (0..2): target velocity (ICL units/s) up to maxForce.
    void setLinearMotor(int axis, bool enable, float targetVel, float maxForce);
    /// Drive rotation on \a axis (0..2): target velocity (rad/s) up to maxForce.
    void setAngularMotor(int axis, bool enable, float targetVel, float maxForce);
    /// Reposition the constraint frames in each body's local space (ICL units).
    void setFrames(const Mat &frameA, const Mat &frameB);
    /// Current relative angle (radians) about \a axis (0..2).
    float getAngle(int axis) const;

  protected:
    struct Data;
    std::unique_ptr<Data> m_data;
    Constraint();                   ///< factory-only (PhysicsWorld)
    void removeFromWorld();         ///< detach + delete Bullet objects, go inert
    bool references(const void *body) const;  ///< does it reference this btRigidBody?
    friend class PhysicsWorld;
  };

  /// Binds one body to a point in space with a 6-axis spring (the legacy
  /// `Object2PointConstraint`). Generalizes to marker-driven manipulation: drive
  /// the spring's target point each frame to pull the body toward a pose.
  /** Wraps `btGeneric6DofSpringConstraint` against a phantom static anchor body
      the constraint owns (and cleans up). Construct via `PhysicsWorld::addSpring`. */
  class ICLPhysics2_API SpringConstraint : public Constraint {
  public:
    /// Move the world-space target point the body is pulled toward (ICL units).
    void setPoint(const Vec &worldPoint);
    /// Move the spring's attachment in the body's local space (ICL units).
    void setLocalOffset(const Vec &localOffset);
    /// Spring stiffness (applied to all 3 linear axes).
    void setStiffness(float k);
    /// Spring damping (applied to all 6 axes).
    void setDamping(float d);

  private:
    SpringConstraint();             ///< factory-only (PhysicsWorld)
    friend class PhysicsWorld;
  };

} // namespace icl::physics2
