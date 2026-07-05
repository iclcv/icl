// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/math/Types.h>
#include <LinearMath/btTransform.h>

#ifndef ICLPhysics2_API
#define ICLPhysics2_API
#endif

namespace icl::physics2 {

  using Mat = math::Mat4;
  using Vec = math::Vec4;

  /// Per-world unit/scale policy: ICL (millimetres) <-> Bullet units.
  /** Replaces the legacy global `ICL_UNIT_TO_METER` / `METER_TO_BULLET_UNIT`
      #defines with an explicit, per-world factor. Default 0.01 reproduces the
      legacy scale (1 ICL mm = 0.01 Bullet units, i.e. ICL mm -> metres ->
      decimetre-based Bullet units). Rotations are never scaled; only lengths,
      positions, velocities and forces are. */
  struct Units {
    float iclToBullet = 0.01f;

    float  toBullet(float x) const { return x * iclToBullet; }
    float  toIcl(float x)    const { return x / iclToBullet; }

    btVector3 toBulletVec(const Vec &v) const {
      return btVector3(v[0]*iclToBullet, v[1]*iclToBullet, v[2]*iclToBullet);
    }
    Vec toIclVec(const btVector3 &v) const {
      float s = 1.0f / iclToBullet;
      return Vec(v[0]*s, v[1]*s, v[2]*s, 1.0f);
    }

    /// ICL transform -> Bullet transform (rotation kept, translation scaled)
    btTransform toBullet(const Mat &m) const {
      btTransform T;
      T.setBasis(btMatrix3x3(m[0], m[1], m[2],
                             m[4], m[5], m[6],
                             m[8], m[9], m[10]));
      T.setOrigin(btVector3(toBullet(m[3]), toBullet(m[7]), toBullet(m[11])));
      return T;
    }

    /// Bullet transform -> ICL transform (rotation kept, translation unscaled)
    Mat toIcl(const btTransform &T) const {
      const btMatrix3x3 &R = T.getBasis();
      const btVector3 &t = T.getOrigin();
      return Mat(R[0][0], R[0][1], R[0][2], toIcl(t[0]),
                 R[1][0], R[1][1], R[1][2], toIcl(t[1]),
                 R[2][0], R[2][1], R[2][2], toIcl(t[2]),
                 0, 0, 0, 1);
    }
  };

} // namespace icl::physics2
