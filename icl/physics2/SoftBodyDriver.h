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

class btSoftBody;

namespace icl::physics2 {

  class PhysicsWorld;
  class RigidBodyDriver;

  /// A geom2 Driver that binds a Bullet soft body (cloth/patch) to a MeshNode.
  /** Soft bodies have no rigid pose — their geometry IS the state. The driver
      builds a btSoftBody patch, registers a post-step capture hook (the sim
      thread snapshots node positions into a SoftStateBuffer), and on the UI
      thread copies that snapshot into its MeshNode each `sync` (recomputing
      normals). The node's transform stays identity; vertices carry world
      positions in ICL units.

      Host node MUST be a geom2::MeshNode. Born from
      PhysicsScene::addCloth(...) / world.addDriver<SoftBodyDriver>(meshNode,...). */
  class ICLPhysics2_API SoftBodyDriver : public geom2::Driver {
  public:
    /// Rectangular cloth patch spanned by four corners (ICL units), with
    /// resX*resY nodes. fixedCornerMask pins corners (1=c00,2=c10,4=c01,8=c11).
    SoftBodyDriver(PhysicsWorld &world,
                   const Vec &c00, const Vec &c10, const Vec &c01, const Vec &c11,
                   int resX, int resY, int fixedCornerMask, float totalMass);
    ~SoftBodyDriver() override;

    void onAttach() override;
    void onDetach() override;
    void sync(double dt, double alpha) override;

    btSoftBody *softBody() const;

    /// Grid node index of a corner (0=c00, 1=c10, 2=c01, 3=c11).
    int cornerNodeIndex(int corner) const;

    /// Pin a soft-body node to a rigid body (anchor). The node then follows the
    /// body — pin a cloth corner to a gripper / kinematic mover. influence 1 =
    /// hard follow. Routed through the world command queue (sim-thread safe).
    void anchorNode(int nodeIndex, RigidBodyDriver *rigid, float influence = 1.0f);

  private:
    struct Data;
    std::unique_ptr<Data> m_data;
  };

} // namespace icl::physics2
