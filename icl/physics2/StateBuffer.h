// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/physics2/Units.h>
#include <LinearMath/btTransform.h>
#include <mutex>
#include <vector>

#ifndef ICLPhysics2_API
#define ICLPhysics2_API
#endif

namespace icl::physics2 {

  /// One body's published pose — the physics<->render membrane, per body.
  /** The sim thread publishes a body's world transform each step; the render
      thread samples it (optionally interpolated) to drive a geom2 node. This
      is the *only* shared mutable state crossing the thread boundary, so the
      golden rule holds: the sim thread never touches a geom2 node, the render
      thread never touches Bullet.

      Double-buffered (prev + curr) so the render side can interpolate between
      the two most recent steps by `alpha in [0,1]`, decoupling visual
      smoothness from the fixed simulation rate. A small mutex guards the swap;
      Phase 1 keeps it simple (a future optimization may swap in a seqlock /
      triple buffer). */
  class ICLPhysics2_API StateSlot {
  public:
    StateSlot() = default;

    /// Publish a new pose (sim thread). First publish seeds both buffers.
    void publish(const btTransform &t) {
      std::scoped_lock lock(m_mutex);
      if (!m_valid) { m_prev = m_curr = t; m_valid = true; }
      else { m_prev = m_curr; m_curr = t; }
    }

    /// Sample the pose (render thread). alpha>=1 returns the latest step;
    /// 0<=alpha<1 interpolates from prev->curr (position lerp, rotation slerp).
    btTransform sample(float alpha = 1.0f) const {
      std::scoped_lock lock(m_mutex);
      if (!m_valid) return btTransform::getIdentity();
      if (alpha >= 1.0f) return m_curr;
      btVector3 o = m_prev.getOrigin().lerp(m_curr.getOrigin(), alpha);
      btQuaternion q = m_prev.getRotation().slerp(m_curr.getRotation(), alpha);
      return btTransform(q, o);
    }

    bool valid() const {
      std::scoped_lock lock(m_mutex);
      return m_valid;
    }

  private:
    mutable std::mutex m_mutex;
    btTransform m_prev, m_curr;
    bool m_valid = false;
  };

  /// A soft body's published node positions (ICL units, world space).
  /** The soft-body analogue of StateSlot: the sim thread snapshots btSoftBody
      node positions here after each step; the render thread copies them into a
      MeshNode. No interpolation in Phase 1/3 (latest wins). */
  class ICLPhysics2_API SoftStateBuffer {
  public:
    void publish(std::vector<Vec> positions) {
      std::scoped_lock lock(m_mutex);
      m_positions = std::move(positions);
      m_valid = true;
    }

    bool sample(std::vector<Vec> &out) const {
      std::scoped_lock lock(m_mutex);
      if (!m_valid) return false;
      out = m_positions;
      return true;
    }

  private:
    mutable std::mutex m_mutex;
    std::vector<Vec> m_positions;
    bool m_valid = false;
  };

} // namespace icl::physics2
