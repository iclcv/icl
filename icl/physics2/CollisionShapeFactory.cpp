// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/physics2/CollisionShapeFactory.h>
#include <icl/geom2/Node.h>
#include <icl/geom2/GeometryNode.h>
#include <icl/geom2/CuboidNode.h>
#include <icl/geom2/SphereNode.h>

#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <BulletCollision/CollisionShapes/btSphereShape.h>
#include <BulletCollision/CollisionShapes/btConvexHullShape.h>

namespace icl::physics2 {

  btCollisionShape *shapeFromNode(const geom2::Node *node, const Units &u) {
    if (!node) return nullptr;

    // Exact primitives for the parametric shapes ICL physics uses most.
    if (auto *cub = dynamic_cast<const geom2::CuboidNode*>(node)) {
      Vec e = cub->getExtents();   // full extents
      return new btBoxShape(btVector3(u.toBullet(e[0]*0.5f),
                                      u.toBullet(e[1]*0.5f),
                                      u.toBullet(e[2]*0.5f)));
    }
    if (auto *sph = dynamic_cast<const geom2::SphereNode*>(node)) {
      return new btSphereShape(u.toBullet(sph->getRadius()));
    }

    // Everything else with geometry: convex hull from its vertices
    // (cylinders, cones, free-form meshes). Vertices are local/origin-centred.
    if (auto *geo = dynamic_cast<const geom2::GeometryNode*>(node)) {
      const auto &verts = geo->getVertices();
      if (verts.empty()) return nullptr;
      auto *hull = new btConvexHullShape();
      for (const auto &v : verts) {
        hull->addPoint(btVector3(u.toBullet(v[0]), u.toBullet(v[1]),
                                 u.toBullet(v[2])), false);
      }
      hull->recalcLocalAabb();
      return hull;
    }

    return nullptr;
  }

} // namespace icl::physics2
