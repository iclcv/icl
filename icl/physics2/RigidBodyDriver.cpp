// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/physics2/RigidBodyDriver.h>
#include <icl/physics2/PhysicsWorld.h>
#include <icl/physics2/StateBuffer.h>
#include <icl/physics2/CollisionShapeFactory.h>
#include <icl/viz3d/nodes/Node.h>
#include <icl/utils/Macros.h>

#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <LinearMath/btMotionState.h>

namespace icl::physics2 {

  namespace {
    /// MotionState = the membrane writer. Bullet calls setWorldTransform on the
    /// sim thread after each step for active dynamic bodies → publish to slot.
    /// getWorldTransform returns our target (used at init, and for kinematic
    /// bodies in Phase 2).
    struct DriverMotionState : public btMotionState {
      StateSlot *slot;
      btTransform target;
      DriverMotionState(const btTransform &t, StateSlot *s) : slot(s), target(t) {}
      void getWorldTransform(btTransform &w) const override { w = target; }
      void setWorldTransform(const btTransform &w) override { target = w; slot->publish(w); }
    };
  }

  struct RigidBodyDriver::Data {
    PhysicsWorld &world;
    float mass;
    Units units;
    StateSlot slot;
    btCollisionShape *shape = nullptr;
    btRigidBody *body = nullptr;
    DriverMotionState *motion = nullptr;
    bool added = false;
    Data(PhysicsWorld &w, float m) : world(w), mass(m) {}
  };

  RigidBodyDriver::RigidBodyDriver(PhysicsWorld &world, float mass)
    : m_data(std::make_unique<Data>(world, mass)) {}

  RigidBodyDriver::~RigidBodyDriver() {
    // onDetach() normally cleaned up already; guard against leaks if not.
    delete m_data->body;
    deleteShape(m_data->shape);
    delete m_data->motion;
  }

  void RigidBodyDriver::onAttach() {
    auto *n = node();
    if (!n) return;
    m_data->units = m_data->world.getUnits();

    m_data->shape = shapeFromNode(n, m_data->units);
    if (!m_data->shape) {
      ERROR_LOG("RigidBodyDriver: node carries no usable geometry for a collision shape");
      return;
    }

    btTransform T = m_data->units.toBullet(n->getTransformation(true));
    m_data->motion = new DriverMotionState(T, &m_data->slot);

    btVector3 inertia(0, 0, 0);
    if (m_data->mass > 0) m_data->shape->calculateLocalInertia(m_data->mass, inertia);
    btRigidBody::btRigidBodyConstructionInfo ci(m_data->mass, m_data->motion,
                                                m_data->shape, inertia);
    m_data->body = new btRigidBody(ci);
    m_data->body->setUserPointer(static_cast<viz3d::Driver *>(this));

    m_data->slot.publish(T);            // seed so the render side has a pose
    m_data->world.addBody(m_data->body);
    m_data->added = true;
  }

  void RigidBodyDriver::onDetach() {
    if (m_data->added && m_data->body) {
      m_data->world.removeBody(m_data->body);
      m_data->added = false;
    }
    delete m_data->body;   m_data->body = nullptr;
    deleteShape(m_data->shape);  m_data->shape = nullptr;
    delete m_data->motion; m_data->motion = nullptr;
  }

  void RigidBodyDriver::sync(double /*dt*/, double alpha) {
    if (auto *n = node()) {
      n->setTransformation(m_data->units.toIcl(m_data->slot.sample((float)alpha)));
    }
  }

  Mat RigidBodyDriver::getPose() const {
    return m_data->units.toIcl(m_data->slot.sample(1.0f));
  }

  btRigidBody *RigidBodyDriver::body() const { return m_data->body; }

  // --- kinematic control ---

  void RigidBodyDriver::setKinematic(bool on) {
    if (!m_data->body) return;
    std::scoped_lock lock(m_data->world);   // serialize against the sim thread
    int flags = m_data->body->getCollisionFlags();
    if (on) {
      m_data->body->setMassProps(0, btVector3(0, 0, 0));
      m_data->body->setCollisionFlags(flags | btCollisionObject::CF_KINEMATIC_OBJECT);
      m_data->body->setActivationState(DISABLE_DEACTIVATION);
    } else {
      m_data->body->setCollisionFlags(flags & ~btCollisionObject::CF_KINEMATIC_OBJECT);
      m_data->body->forceActivationState(ACTIVE_TAG);
    }
  }

  void RigidBodyDriver::setKinematicTransform(const Mat &m) {
    btTransform T = m_data->units.toBullet(m);
    DriverMotionState *motion = m_data->motion;
    StateSlot *slot = &m_data->slot;
    // Sim thread: Bullet reads the MotionState target each substep to move the
    // kinematic body; it does NOT call setWorldTransform back, so publish the
    // slot ourselves so the render side follows.
    m_data->world.enqueue([motion, slot, T]() {
      if (motion) motion->target = T;
      slot->publish(T);
    });
  }

  void RigidBodyDriver::setTransform(const Mat &m) {
    btTransform T = m_data->units.toBullet(m);
    btRigidBody *body = m_data->body;
    StateSlot *slot = &m_data->slot;
    // Teleport a (dynamic) body: reposition + drop all motion. Routed through the
    // command queue so it lands between steps; publish the slot for the render side.
    m_data->world.enqueue([body, slot, T]() {
      if (!body) return;
      body->setWorldTransform(T);
      body->setLinearVelocity(btVector3(0, 0, 0));
      body->setAngularVelocity(btVector3(0, 0, 0));
      body->clearForces();
      body->activate(true);
      slot->publish(T);
    });
  }

  // --- tunables ---

  void RigidBodyDriver::setMass(float mass) {
    m_data->mass = mass;
    if (!m_data->body || !m_data->shape) return;
    btVector3 inertia(0, 0, 0);
    if (mass > 0) m_data->shape->calculateLocalInertia(mass, inertia);
    std::scoped_lock lock(m_data->world);   // serialize against the sim thread
    m_data->body->setMassProps(mass, inertia);
  }

  void RigidBodyDriver::setFriction(float f) {
    if (m_data->body) m_data->body->setFriction(f);
  }
  void RigidBodyDriver::setRestitution(float r) {
    if (m_data->body) m_data->body->setRestitution(r);
  }
  void RigidBodyDriver::setRollingFriction(float rf) {
    if (m_data->body) m_data->body->setRollingFriction(rf);
  }
  void RigidBodyDriver::setDamping(float linear, float angular) {
    if (m_data->body) m_data->body->setDamping(linear, angular);
  }
  void RigidBodyDriver::setLinearVelocity(const Vec &v) {
    if (m_data->body) {
      m_data->body->setLinearVelocity(m_data->units.toBulletVec(v));
      m_data->body->activate();
    }
  }
  void RigidBodyDriver::setAngularVelocity(const Vec &v) {
    if (m_data->body) {
      m_data->body->setAngularVelocity(btVector3(v[0], v[1], v[2]));  // unscaled (rad/s)
      m_data->body->activate();
    }
  }
  Vec RigidBodyDriver::getLinearVelocity() const {
    if (!m_data->body) return Vec(0, 0, 0, 1);
    return m_data->units.toIclVec(m_data->body->getLinearVelocity());   // ICL units/s
  }
  void RigidBodyDriver::applyCentralForce(const Vec &f) {
    if (m_data->body) {
      m_data->body->applyCentralForce(m_data->units.toBulletVec(f));
      m_data->body->activate();
    }
  }

  void RigidBodyDriver::setCcd(float motionThreshold, float sweptSphereRadius) {
    if (!m_data->body) return;
    std::scoped_lock lock(m_data->world);
    m_data->body->setCcdMotionThreshold(m_data->units.toBullet(motionThreshold));
    m_data->body->setCcdSweptSphereRadius(m_data->units.toBullet(sweptSphereRadius));
  }

  void RigidBodyDriver::setCollisionFilter(int group, int mask) {
    if (!m_data->body) return;
    btRigidBody *body = m_data->body;
    PhysicsWorld *world = &m_data->world;
    world->enqueue([world, body, group, mask]() { world->setBodyFilter(body, group, mask); });
  }

} // namespace icl::physics2
