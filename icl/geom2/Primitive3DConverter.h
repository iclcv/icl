// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/geom2/Node.h>
#include <icl/geom/Primitive3DFilter.h>
#include <icl/cv3d/GeomDefs.h>
#include <memory>

namespace icl::geom2 {

  /// Convert a geom Primitive3DFilter::Primitive3D into a geom2 node.
  /** The geom2 replacement for Primitive3DFilter::Primitive3D::toSceneObject:
      maps CUBE→CuboidNode, SPHERE→SphereNode, CYLINDER→CylinderNode (built at
      the origin from the primitive's scale), then applies the primitive's
      orientation+position as the node transform and the given material colour.
      Returns nullptr for an unknown primitive type. */
  ICLGeom2_API NodePtr nodeFromPrimitive3D(
      const geom::Primitive3DFilter::Primitive3D &p,
      uint32_t slices = 15,
      const geom::GeomColor &color = geom::geom_white(100));

} // namespace icl::geom2
