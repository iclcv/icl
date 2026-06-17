// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/geom2/Driver.h>
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
  class ICLPhysics2_API SensorDriver : public geom2::Driver {
  public:
    explicit SensorDriver(PhysicsWorld &world);
    ~SensorDriver() override;

    void onAttach() override;
    void onDetach() override;
    void sync(double dt, double alpha) override;

    /// Drivers whose bodies currently overlap the zone.
    std::vector<geom2::Driver *> getOverlappingDrivers() const;

  private:
    struct Data;
    std::unique_ptr<Data> m_data;
  };

} // namespace icl::physics2
