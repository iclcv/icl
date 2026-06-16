// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/physics/PhysicsWorld.h>
#include <icl/geom2/Scene2.h>
#include <icl/geom2/Node.h>
#include <icl/geom2/MeshNode.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom/Camera.h>
#include <icl/qt/GLCallback.h>
#include <memory>
#include <vector>

namespace icl::physics {

  class PhysicsObject;

  /// Convenience scene that simulates in a PhysicsWorld and renders via geom2.
  /** Drop-in successor to PhysicsScene for the geom→geom2 migration. Where the
      legacy PhysicsScene multiply-inherited the (now dead) geom::Scene GL path,
      PhysicsScene2 owns a geom2::Scene2 and mirrors physics objects into it:

      - addObject() adds the object to the PhysicsWorld AND auto-creates a
        geom2 mirror node for it (via geom2::fromSceneObject) in the Scene2.
      - syncSceneFromPhysics() — called once per frame after step() — runs each
        object's Bullet→SceneObject update (PhysicsObject::prepareForRendering)
        and copies the result into its mirror node: rigid bodies update only
        their transform; deformable (soft) bodies re-upload their mesh, which
        marks just that node dirty (see GeometryNode::markGeometryDirty) so the
        renderer re-uploads only the changed node.

      Composition (not multiple inheritance) is used so Scene2::lock() and
      PhysicsWorld::lock() don't collide. PhysicsWorld methods (step, setGravity,
      addConstraint, rayCast, …) are available directly; the common Scene2
      methods are forwarded, and getScene2() is the full escape hatch. */
  class ICLPhysics_API PhysicsScene2 : public PhysicsWorld {
    public:
    PhysicsScene2(PhysicsWorld::BulletSolverType type = PhysicsWorld::Default);
    ~PhysicsScene2();

    /// add a physics object (to the world) and its geom2 mirror node (to the scene)
    /** Ownership of the object is not taken (matches PhysicsWorld::addObject). */
    void addObject(PhysicsObject *obj);

    /// remove a physics object and its mirror node
    void removeObject(PhysicsObject *obj);

    /// per-frame Bullet→SceneObject→geom2 sync; call after step(), before render
    void syncSceneFromPhysics();

    // --- geom2 scene access ---
    geom2::Scene2 &getScene2();
    const geom2::Scene2 &getScene2() const;

    // --- common Scene2 forwarders (use getScene2() for the rest) ---
    void addCamera(const geom::Camera &cam);
    geom::Camera &getCamera(int i);
    void setBounds(float maxDim);
    /// add a non-physics decoration node (ground, light, markers, …)
    void addNode(std::shared_ptr<geom2::Node> node);
    void addLight(std::shared_ptr<geom2::LightNode> light);
    std::shared_ptr<qt::GLCallback> getGLCallback(int cameraIndex);

    private:
    struct Mirror {
      PhysicsObject *obj;
      std::shared_ptr<geom2::Node> node;
      std::shared_ptr<geom2::MeshNode> mesh;  // non-null iff deformable leaf
      bool deformable;
    };
    geom2::Scene2 m_scene;
    std::vector<Mirror> m_mirrors;
  };

} // namespace icl::physics
