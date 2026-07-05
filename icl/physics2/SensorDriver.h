// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/viz3d/scene/Driver.h>
#include <icl/physics2/Units.h>
#include <memory>
#include <vector>

#ifndef ICLPhysics2_API
#define ICLPhysics2_API
#endif

namespace icl::physics2 {

  class PhysicsWorld;

  /// A trigger zone: detects overlapping bodies without any collision response.
  /** Wraps a Bullet ghost object whose collision shape is derived from the host
      node's geometry (so the visible node IS the zone — set it invisible for an
      invisible trigger). `getOverlappingDrivers()` returns the drivers currently
      inside the zone. Bodies pass through untouched.

      Threading: query `getOverlappingDrivers()` between steps (the ghost's
      overlap cache is updated on the sim thread during a step). */
  class ICLPhysics2_API SensorDriver : public viz3d::Driver {
  public:
    explicit SensorDriver(PhysicsWorld &world);
    ~SensorDriver() override;

    void onAttach() override;
    void onDetach() override;
    void sync(double dt, double alpha) override;

    /// Move the zone to a new world pose (ICL units), e.g. to make it follow a
    /// kinematic body. Routed through the command queue (sim-thread safe). The
    /// host node's visual is the caller's responsibility (set it to match).
    void setTransform(const Mat &worldPose);

    /// Drivers whose bodies currently overlap the zone.
    std::vector<viz3d::Driver *> getOverlappingDrivers() const;

  private:
    struct Data;
    std::unique_ptr<Data> m_data;
  };

} // namespace icl::physics2
