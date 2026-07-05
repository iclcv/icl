// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/physics2/Units.h>

#ifndef ICLPhysics2_API
#define ICLPhysics2_API
#endif

class btCollisionShape;

namespace icl::viz3d { class Node; }

namespace icl::physics2 {

  /// Derive a Bullet collision shape from a viz3d node's geometry.
  /** Convention (single source of truth): the node's geometry is centred at its
      local origin and its pose lives in the node transform — so the derived
      shape is centred at the body origin and matches the rendered geometry.

      - GroupNode   -> btCompoundShape (each child at its local transform)
      - CuboidNode  -> btBoxShape (from extents)
      - SphereNode  -> btSphereShape (from radius)
      - any other GeometryNode -> btConvexHullShape (from its vertices)

      Returns nullptr if the node carries no usable geometry. Lengths are scaled
      to Bullet units via `units`. */
  ICLPhysics2_API btCollisionShape *shapeFromNode(const viz3d::Node *node,
                                                  const Units &units);

  /// Free a shape from shapeFromNode. Recurses into btCompoundShape children
  /// (Bullet does not own them), so it is the correct counterpart for all shapes.
  ICLPhysics2_API void deleteShape(btCollisionShape *shape);

} // namespace icl::physics2
