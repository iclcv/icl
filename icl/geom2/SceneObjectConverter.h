// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/geom2/Node.h>
#include <icl/geom2/MeshNode.h>
#include <memory>

namespace icl::geom {
  class SceneObject;
}

namespace icl::geom2 {

  /// Build a geom2 node tree mirroring a legacy geom::SceneObject.
  /** A single universal converter for the geom→geom2 migration: it walks the
      SceneObject's shared vertices/normals/colors/texcoords plus its
      polymorphic primitive list (lines, triangles, quads, polygons) and
      emits the equivalent geom2 geometry into a MeshNode. Children recurse
      into a GroupNode, carrying each object's local transform and visibility.

      This covers every shape the physics module produces (box/sphere/cylinder
      primitives, convex hulls, soft-body OBJ meshes, and compound objects),
      so the physics→geom2 render bridge needs no per-type mapping.

      Limitations (acceptable for the migration, may be extended later):
      - faces are shaded with a single node Material (the first face
        primitive's colour/material); per-face colours collapse to one.
      - texture/text primitives are skipped (geometry only, untextured).
      - per-vertex face colours are not rendered (geom2 shades faces by
        material); vertex colours are kept for point rendering. */
  ICLGeom2_API std::shared_ptr<Node> fromSceneObject(const geom::SceneObject &so);

  /// Convert only the object's own geometry into a MeshNode (no children,
  /// no transform). Useful when the caller manages the node hierarchy and
  /// transforms itself (e.g. the physics bridge syncing Bullet poses).
  ICLGeom2_API std::shared_ptr<MeshNode> meshFromSceneObject(const geom::SceneObject &so);

} // namespace icl::geom2
