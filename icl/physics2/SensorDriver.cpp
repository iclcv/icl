// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/physics2/SensorDriver.h>
#include <icl/physics2/PhysicsWorld.h>
#include <icl/physics2/CollisionShapeFactory.h>
#include <icl/viz3d/nodes/Node.h>
#include <icl/utils/Macros.h>

#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <BulletCollision/BroadphaseCollision/btBroadphaseProxy.h>

namespace icl::physics2 {

  struct SensorDriver::Data {
    PhysicsWorld &world;
    Units units;
    btPairCachingGhostObject *ghost = nullptr;
    btCollisionShape *shape = nullptr;
    bool added = false;
    explicit Data(PhysicsWorld &w) : world(w) {}
  };

  SensorDriver::SensorDriver(PhysicsWorld &world)
    : m_data(std::make_unique<Data>(world)) {}

  SensorDriver::~SensorDriver() {
    delete m_data->ghost;
    deleteShape(m_data->shape);
  }

  void SensorDriver::onAttach() {
    auto *n = node();
    if (!n) return;
    m_data->units = m_data->world.getUnits();
    m_data->shape = shapeFromNode(n, m_data->units);
    if (!m_data->shape) { ERROR_LOG("SensorDriver: node has no usable geometry"); return; }

    auto *ghost = new btPairCachingGhostObject();
    ghost->setCollisionShape(m_data->shape);
    ghost->setWorldTransform(m_data->units.toBullet(n->getTransformation(true)));
    ghost->setCollisionFlags(ghost->getCollisionFlags() |
                             btCollisionObject::CF_NO_CONTACT_RESPONSE);
    ghost->setUserPointer(static_cast<viz3d::Driver *>(this));
    m_data->ghost = ghost;

    // group 1, mask all -> overlaps every default body without responding
    m_data->world.addCollisionObject(ghost, 1, -1);
    m_data->added = true;
  }

  void SensorDriver::onDetach() {
    if (m_data->added && m_data->ghost) {
      m_data->world.removeCollisionObject(m_data->ghost);
      m_data->added = false;
    }
    delete m_data->ghost;  m_data->ghost = nullptr;
    deleteShape(m_data->shape);  m_data->shape = nullptr;
  }

  void SensorDriver::sync(double, double) {
    // Static zone: the host node already carries the visual; nothing to pull.
    // (A moving sensor would update the ghost transform here from the node.)
  }

  void SensorDriver::setTransform(const Mat &worldPose) {
    const btTransform T = m_data->units.toBullet(worldPose);
    btPairCachingGhostObject *ghost = m_data->ghost;
    m_data->world.enqueue([ghost, T]() { if (ghost) ghost->setWorldTransform(T); });
  }

  std::vector<viz3d::Driver *> SensorDriver::getOverlappingDrivers() const {
    std::vector<viz3d::Driver *> out;
    if (!m_data->ghost) return out;
    const int n = m_data->ghost->getNumOverlappingObjects();
    for (int i = 0; i < n; i++) {
      btCollisionObject *o = m_data->ghost->getOverlappingObject(i);
      if (!o) continue;
      if (auto *d = static_cast<viz3d::Driver *>(o->getUserPointer())) out.push_back(d);
    }
    return out;
  }

} // namespace icl::physics2
