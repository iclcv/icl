// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/physics2/CollisionShapeFactory.h>
#include <icl/viz3d/Node.h>
#include <icl/viz3d/GroupNode.h>
#include <icl/viz3d/GeometryNode.h>
#include <icl/viz3d/CuboidNode.h>
#include <icl/viz3d/SphereNode.h>

#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <BulletCollision/CollisionShapes/btSphereShape.h>
#include <BulletCollision/CollisionShapes/btConvexHullShape.h>
#include <BulletCollision/CollisionShapes/btCompoundShape.h>

namespace icl::physics2 {

  btCollisionShape *shapeFromNode(const viz3d::Node *node, const Units &u) {
    if (!node) return nullptr;

    // A GroupNode becomes ONE compound body: each child contributes its shape at
    // the child's local transform (relative to the group origin). Recurses, so a
    // group of groups works too. The maze board and the rocket bottle are built
    // this way — many primitives moving as a single rigid (or kinematic) body.
    if (auto *grp = dynamic_cast<const viz3d::GroupNode*>(node)) {
      auto *compound = new btCompoundShape();
      for (int i = 0; i < grp->getChildCount(); i++) {
        const viz3d::Node *child = grp->getChild(i);
        if (btCollisionShape *cs = shapeFromNode(child, u))
          compound->addChildShape(u.toBullet(child->getTransformation(false)), cs);
      }
      if (compound->getNumChildShapes() == 0) { delete compound; return nullptr; }
      return compound;
    }

    // Exact primitives for the parametric shapes ICL physics uses most.
    if (auto *cub = dynamic_cast<const viz3d::CuboidNode*>(node)) {
      Vec e = cub->getExtents();   // full extents
      return new btBoxShape(btVector3(u.toBullet(e[0]*0.5f),
                                      u.toBullet(e[1]*0.5f),
                                      u.toBullet(e[2]*0.5f)));
    }
    if (auto *sph = dynamic_cast<const viz3d::SphereNode*>(node)) {
      return new btSphereShape(u.toBullet(sph->getRadius()));
    }

    // Everything else with geometry: convex hull from its vertices
    // (cylinders, cones, free-form meshes). Vertices are local/origin-centred.
    if (auto *geo = dynamic_cast<const viz3d::GeometryNode*>(node)) {
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

  void deleteShape(btCollisionShape *shape) {
    if (!shape) return;
    // btCompoundShape doesn't own its children — free them first, recursively.
    if (auto *compound = dynamic_cast<btCompoundShape*>(shape)) {
      for (int i = 0; i < compound->getNumChildShapes(); i++)
        deleteShape(compound->getChildShape(i));
    }
    delete shape;
  }

} // namespace icl::physics2
