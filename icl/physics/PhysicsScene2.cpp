// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/physics/PhysicsScene2.h>
#include <icl/physics/PhysicsObject.h>
#include <icl/physics/SoftObject.h>
#include <icl/geom2/SceneObjectConverter.h>
#include <algorithm>

namespace icl::physics {

  PhysicsScene2::PhysicsScene2(PhysicsWorld::BulletSolverType type)
      : PhysicsWorld(type) {
    // The legacy geom::Scene provided default lighting; geom2::Scene2 does not.
    // Add a sensible default so migrated demos are lit out of the box. The PBR
    // ambient comes from a fairly bright hard-coded sky (~0.6·albedo), leaving
    // little headroom, so keep the key light moderate and pull exposure below 1
    // to stop lighter surfaces blowing out. Point lights here don't attenuate
    // with distance. Override any of this via getScene2().
    m_scene.getRenderer().setExposure(0.75f);
    auto light = std::make_shared<geom2::LightNode>(geom2::LightNode::Point);
    light->setIntensity(0.6f);
    light->translate(2000, 1000, 4000);
    light->setShadowEnabled(true);
    light->setSoftShadowRadius(3.0f);
    m_scene.addLight(light);
  }

  PhysicsScene2::~PhysicsScene2() {
    for (Mirror &m : m_mirrors) {
      if (m.owned) { PhysicsWorld::removeObject(m.obj); delete m.obj; }
    }
  }

  void PhysicsScene2::addObject(PhysicsObject *obj, bool passOwnership) {
    if (!obj) return;
    PhysicsWorld::addObject(obj);
    obj->prepareForRendering();  // initial Bullet→SceneObject sync

    auto node = geom2::fromSceneObject(*obj);
    m_scene.addNode(node);

    Mirror m;
    m.obj = obj;
    m.node = node;
    m.deformable = (dynamic_cast<SoftObject *>(obj) != nullptr);
    if (m.deformable) m.mesh = std::dynamic_pointer_cast<geom2::MeshNode>(node);
    m.owned = passOwnership;
    m_mirrors.push_back(std::move(m));
  }

  void PhysicsScene2::removeObject(PhysicsObject *obj) {
    PhysicsWorld::removeObject(obj);
    auto it = std::find_if(m_mirrors.begin(), m_mirrors.end(),
                           [obj](const Mirror &m) { return m.obj == obj; });
    if (it != m_mirrors.end()) {
      m_scene.removeNode(it->node.get());
      bool owned = it->owned;
      PhysicsObject *o = it->obj;
      m_mirrors.erase(it);
      if (owned) delete o;
    }
  }

  void PhysicsScene2::syncSceneFromPhysics() {
    for (Mirror &m : m_mirrors) {
      m.obj->prepareForRendering();  // Bullet→SceneObject (locks internally)
      m.node->setTransformation(m.obj->getTransformation());
      if (m.deformable && m.mesh) {
        // soft-body vertices live in world space; re-upload only this node
        auto &dst = m.mesh->getVertices();  // bumps geometry version
        const std::vector<geom2::Vec> &src = m.obj->getVertices();
        if (dst.size() == src.size()) std::copy(src.begin(), src.end(), dst.begin());
        else dst = src;
        m.mesh->createAutoNormals(true);
      }
    }
  }

  geom2::Scene2 &PhysicsScene2::getScene2() { return m_scene; }
  const geom2::Scene2 &PhysicsScene2::getScene2() const { return m_scene; }

  void PhysicsScene2::addCamera(const geom::Camera &cam) { m_scene.addCamera(cam); }
  geom::Camera &PhysicsScene2::getCamera(int i) { return m_scene.getCamera(i); }
  void PhysicsScene2::setBounds(float maxDim) { m_scene.setBounds(maxDim); }
  void PhysicsScene2::addNode(std::shared_ptr<geom2::Node> node) { m_scene.addNode(std::move(node)); }
  void PhysicsScene2::addLight(std::shared_ptr<geom2::LightNode> light) { m_scene.addLight(std::move(light)); }
  std::shared_ptr<qt::GLCallback> PhysicsScene2::getGLCallback(int cameraIndex) {
    return m_scene.getGLCallback(cameraIndex);
  }

} // namespace icl::physics
