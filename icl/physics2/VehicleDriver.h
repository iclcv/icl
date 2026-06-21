// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/geom2/Driver.h>
#include <icl/geom2/Node.h>
#include <icl/physics2/Units.h>
#include <memory>
#include <vector>

#ifndef ICLPhysics2_API
#define ICLPhysics2_API
#endif

namespace icl::physics2 {

  class PhysicsWorld;

  /// Vehicle parameters. Lengths are ICL units (mm); the wheel connection points
  /// are derived from the chassis AABB (× the *Frac fractions). Suspension
  /// constants are dimensionless Bullet-side tunables.
  struct VehicleConfig {
    float chassisMass = 800.f;        ///< chassis mass
    float wheelRadius = 60.f;         ///< mm
    float wheelWidth = 40.f;          ///< mm (visual only)
    float suspensionRestLength = 80.f;///< mm (natural extension below the connection point)
    float suspensionStiffness = 20.f;
    float suspensionDamping = 2.3f;
    float suspensionCompression = 4.4f;
    float wheelFriction = 1000.f;     ///< high = grippy
    float rollInfluence = 0.1f;       ///< 0..1, lower = less body roll in turns
    float connectionHeight = 0.f;     ///< mm, wheel mount z relative to the chassis bottom
    float trackFrac = 0.95f;          ///< wheel x-spread as a fraction of chassis half-width
    float baseFrac = 0.85f;           ///< wheel y-spread as a fraction of chassis half-length
  };

  /// A geom2 Driver that turns its host node into a `btRaycastVehicle` — a car
  /// with a rigid chassis and four *raycast* wheels (no separate wheel bodies).
  /** Replaces the legacy hinge-wheel car. The host node supplies the chassis
      collision shape (a `CuboidNode` works well); the driver creates four wheel
      `CylinderNode`s the owning scene renders (fetch them with `getWheelNodes()`
      and add them — `PhysicsScene::addVehicle` does this for you).

      Thread split (same membrane pattern as the other drivers): the vehicle's
      `btActionInterface` ticks on the sim thread inside `step()`; a post-step
      capture hook snapshots the chassis + four wheel world transforms into
      `StateSlot`s; `sync()` samples them on the UI thread into the chassis node +
      wheel nodes. Controls are routed through the world command queue so they
      apply *before* the next step (engine force is read during the tick).

      Forward is the chassis-local **+Y**, up **+Z**, right **+X** (Z-up, matching
      the default gravity). Lengths in `Config` are ICL units (mm), scaled to
      Bullet internally; suspension stiffness/damping/friction are dimensionless
      Bullet tunables. Runs in any world (the default unified `Deformable` world
      included). */
  class ICLPhysics2_API VehicleDriver : public geom2::Driver {
  public:
    using Config = VehicleConfig;   ///< back-compat alias

    VehicleDriver(PhysicsWorld &world, const VehicleConfig &cfg = {});
    ~VehicleDriver() override;

    void onAttach() override;
    void onDetach() override;
    void sync(double dt, double alpha) override;

    // --- driving controls (routed through the command queue; sim-thread safe) ---
    /// Engine force on the driven (rear) wheels; sign sets forward/reverse.
    void setEngineForce(float force);
    /// Brake force on all wheels (0 = off).
    void setBrake(float brake);
    /// Front-wheel steering angle in radians (+ = left).
    void setSteering(float radians);

    /// Current forward speed in km/h (unit-scale corrected; sign = direction).
    float getSpeedKmh() const;
    /// Chassis world pose in ICL units (for a chase camera). Samples the slot.
    Mat getChassisPose() const;

    /// The four wheel nodes (FL, FR, RL, RR) the scene must render. Valid after
    /// onAttach(); `PhysicsScene::addVehicle` adds them for you.
    const std::vector<geom2::NodePtr> &getWheelNodes() const;

  private:
    struct Data;
    std::unique_ptr<Data> m_data;
  };

} // namespace icl::physics2
