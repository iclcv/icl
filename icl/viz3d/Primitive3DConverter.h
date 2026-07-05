// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/viz3d/nodes/Node.h>
#include <icl/cv3d/Primitive3D.h>
#include <icl/cv3d/GeomDefs.h>
#include <memory>

namespace icl::viz3d {

  /// Convert a geom Primitive3D into a viz3d node.
  /** The viz3d replacement for Primitive3D::toSceneObject:
      maps CUBE→CuboidNode, SPHERE→SphereNode, CYLINDER→CylinderNode (built at
      the origin from the primitive's scale), then applies the primitive's
      orientation+position as the node transform and the given material colour.
      Returns nullptr for an unknown primitive type. */
  ICLViz3d_API NodePtr nodeFromPrimitive3D(
      const cv3d::Primitive3D &p,
      uint32_t slices = 15,
      const cv3d::GeomColor &color = cv3d::geom_white(100));

} // namespace icl::viz3d
