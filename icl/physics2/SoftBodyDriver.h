// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/config/Configurable.h>
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
  /** Tunable cloth parameters are exposed as live `utils::Configurable`
      properties (stiffness / friction / damping / contact hardness / position
      iterations / collision margin / self collision); changing one applies to
      the running soft body via the world's sim-thread command queue. Drop a
      `Prop(clothDriver)` into a GUI to dial them in interactively. */
  class ICLPhysics2_API SoftBodyDriver : public geom2::Driver,
                                         public utils::Configurable {
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

    /// Reset the cloth to its initial flat patch with zero velocity (sim-thread
    /// safe — routed through the world command queue). Pins are preserved.
    void reset();

  private:
    /// Read the tunable properties and apply them to the body — directly when
    /// \a live is false (initial setup, sim not yet running), else enqueued onto
    /// the world's sim thread.
    void pushConfig(bool live);

    /// Re-lay the nodes onto a flat patch spanned by the four corners, zero
    /// velocity. \a rebakeRest makes the new flat shape the rest state (rest
    /// lengths follow the size); else the original rest lengths are kept (a
    /// pure reset). Sim-thread safe (enqueued).
    void placeFlatPatch(const Vec &c00, const Vec &c10,
                        const Vec &c01, const Vec &c11, bool rebakeRest);

    /// Re-lay + rebake the cloth at the current "size" scale (about its centre).
    void applySize();

    /// The four corners scaled by the current "size" about the patch centre.
    void cornersAtSize(Vec &c00, Vec &c10, Vec &c01, Vec &c11) const;

    /// Create the btSoftBody + mesh topology at the current resolution/size and
    /// register it (assumes no body is currently built).
    void buildBody();

    /// Tear down and rebuild the soft body at a new grid resolution. Locks the
    /// world (blocks the sim thread) so the body swap + mesh-topology rebuild
    /// happen atomically; called from the UI-thread property callback.
    void rebuildAtResolution(int resX, int resY);

    struct Data;
    std::unique_ptr<Data> m_data;
  };

} // namespace icl::physics2
