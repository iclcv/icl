// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/physics2/Constraint.h>
#include <icl/physics2/PhysicsWorld.h>
#include <icl/physics2/RigidBodyDriver.h>
#include <icl/viz3d/nodes/Node.h>
#include <icl/utils/Exception.h>

#include <btBulletDynamicsCommon.h>
#include <BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h>
#include <BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h>
#include <BulletCollision/CollisionShapes/btEmptyShape.h>

#include <memory>

namespace icl::physics2 {

  // ---- PIMPL ----

  struct Constraint::Data {
    PhysicsWorld *world = nullptr;
    btTypedConstraint *constraint = nullptr;     // owned; base type (both kinds)
    btGeneric6DofConstraint *dof = nullptr;      // same object, typed for setters
    btRigidBody *anchor = nullptr;               // spring only: phantom static body
    btCollisionShape *anchorShape = nullptr;     // spring only
    std::weak_ptr<RigidBodyDriver> a, b;         // cross-edges (don't keep bodies alive)
    Units units;
    bool active = true;
  };

  // ---- lifetime (factory-built, world-owned) ----

  Constraint::Constraint() : m_data(std::make_unique<Data>()) {}

  Constraint::~Constraint() {
    // Normally removeFromWorld() already detached us (called by the world on
    // body removal / removeConstraint / world teardown). This is the last-ditch
    // free if a Constraint somehow outlives that path — delete nullptr is a no-op.
    delete m_data->constraint;
    delete m_data->anchor;
    delete m_data->anchorShape;
  }

  bool Constraint::isActive() const { return m_data->active; }

  // Caller (PhysicsWorld) holds the world lock.
  void Constraint::removeFromWorld() {
    if (!m_data->world || !m_data->active) return;
    btDynamicsWorld *w = m_data->world->getDynamicsWorld();
    if (m_data->constraint) {
      w->removeConstraint(m_data->constraint);
      delete m_data->constraint;
      m_data->constraint = nullptr;
      m_data->dof = nullptr;
    }
    if (m_data->anchor) {
      w->removeRigidBody(m_data->anchor);
      delete m_data->anchor;
      m_data->anchor = nullptr;
    }
    delete m_data->anchorShape;
    m_data->anchorShape = nullptr;
    m_data->active = false;
  }

  bool Constraint::references(const void *body) const {
    if (auto a = m_data->a.lock()) if ((const void *)a->body() == body) return true;
    if (auto b = m_data->b.lock()) if ((const void *)b->body() == body) return true;
    return false;
  }

  // ---- tunables (serialized against the sim thread; inert if detached) ----

  void Constraint::setLinearLimits(const Vec &lower, const Vec &upper) {
    if (!m_data->dof) return;
    std::scoped_lock lock(*m_data->world);
    m_data->dof->setLinearLowerLimit(m_data->units.toBulletVec(lower));
    m_data->dof->setLinearUpperLimit(m_data->units.toBulletVec(upper));
  }

  void Constraint::setAngularLimits(const Vec &lower, const Vec &upper) {
    if (!m_data->dof) return;
    std::scoped_lock lock(*m_data->world);
    m_data->dof->setAngularLowerLimit(btVector3(lower[0], lower[1], lower[2]));
    m_data->dof->setAngularUpperLimit(btVector3(upper[0], upper[1], upper[2]));
  }

  void Constraint::setLinearMotor(int axis, bool enable, float targetVel, float maxForce) {
    if (!m_data->dof || axis < 0 || axis > 2) return;
    std::scoped_lock lock(*m_data->world);
    btTranslationalLimitMotor *m = m_data->dof->getTranslationalLimitMotor();
    m->m_enableMotor[axis] = enable;
    m->m_targetVelocity[axis] = m_data->units.toBullet(targetVel);
    m->m_maxMotorForce[axis] = maxForce;
  }

  void Constraint::setAngularMotor(int axis, bool enable, float targetVel, float maxForce) {
    if (!m_data->dof || axis < 0 || axis > 2) return;
    std::scoped_lock lock(*m_data->world);
    btRotationalLimitMotor *m = m_data->dof->getRotationalLimitMotor(axis);
    m->m_enableMotor = enable;
    m->m_targetVelocity = targetVel;        // rad/s, unscaled
    m->m_maxMotorForce = maxForce;
  }

  void Constraint::setFrames(const Mat &frameA, const Mat &frameB) {
    if (!m_data->dof) return;
    std::scoped_lock lock(*m_data->world);
    m_data->dof->setFrames(m_data->units.toBullet(frameA), m_data->units.toBullet(frameB));
  }

  float Constraint::getAngle(int axis) const {
    if (!m_data->dof || axis < 0 || axis > 2) return 0.f;
    std::scoped_lock lock(*m_data->world);
    m_data->dof->calculateTransforms();     // refresh between steps
    return m_data->dof->getAngle(axis);
  }

  // ---- SpringConstraint ----

  SpringConstraint::SpringConstraint() = default;

  void SpringConstraint::setPoint(const Vec &worldPoint) {
    if (!m_data->dof) return;
    std::scoped_lock lock(*m_data->world);
    static_cast<btGeneric6DofSpringConstraint *>(m_data->dof)
        ->getFrameOffsetA().setOrigin(m_data->units.toBulletVec(worldPoint));
  }

  void SpringConstraint::setLocalOffset(const Vec &localOffset) {
    if (!m_data->dof) return;
    std::scoped_lock lock(*m_data->world);
    static_cast<btGeneric6DofSpringConstraint *>(m_data->dof)
        ->getFrameOffsetB().setOrigin(m_data->units.toBulletVec(localOffset));
  }

  void SpringConstraint::setStiffness(float k) {
    if (!m_data->dof) return;
    std::scoped_lock lock(*m_data->world);
    auto *s = static_cast<btGeneric6DofSpringConstraint *>(m_data->dof);
    for (int i = 0; i < 3; i++) s->setStiffness(i, k);
  }

  void SpringConstraint::setDamping(float d) {
    if (!m_data->dof) return;
    std::scoped_lock lock(*m_data->world);
    auto *s = static_cast<btGeneric6DofSpringConstraint *>(m_data->dof);
    for (int i = 0; i < 6; i++) s->setDamping(i, d);
  }

  // ---- factories (PhysicsWorld members; here so Constraint::Data is visible) ----

  namespace {
    btTransform pivotFrame(const Units &u, const Vec &p) {
      btTransform t;
      t.setIdentity();
      t.setOrigin(u.toBulletVec(p));
      return t;
    }
    // free = lower>upper, locked = lower==upper(0)
    void freeAxis(btVector3 &lo, btVector3 &hi, int axis) { lo[axis] = 1; hi[axis] = -1; }

    RigidBodyDriver *resolve(const viz3d::NodePtr &n,
                             std::shared_ptr<RigidBodyDriver> &keep) {
      if (!n) throw utils::ICLException("PhysicsWorld constraint: null node");
      keep = n->getDriverPtr<RigidBodyDriver>();
      if (!keep || !keep->body())
        throw utils::ICLException("PhysicsWorld constraint: node has no rigid body "
                                  "(add it with addRigidBody / scene.add first)");
      return keep.get();
    }
  }

  // Build a 6DOF constraint between two bodies, apply the preset, wire the
  // cross-edges, register it with the world.
  std::shared_ptr<Constraint>
  PhysicsWorld::makeDof(viz3d::NodePtr na, viz3d::NodePtr nb,
                        const Vec &pivA, const Vec &pivB, Joint kind, int axis) {
    std::shared_ptr<RigidBodyDriver> ka, kb;
    RigidBodyDriver *da = resolve(na, ka), *db = resolve(nb, kb);
    std::scoped_lock lock(*this);
    auto c = std::shared_ptr<Constraint>(new Constraint());
    c->m_data->world = this;
    c->m_data->units = getUnits();
    c->m_data->a = ka;
    c->m_data->b = kb;
    auto *dof = new btGeneric6DofConstraint(*da->body(), *db->body(),
                                            pivotFrame(c->m_data->units, pivA),
                                            pivotFrame(c->m_data->units, pivB), true);
    dof->setLinearLowerLimit(btVector3(0, 0, 0));   // all locked by default
    dof->setLinearUpperLimit(btVector3(0, 0, 0));
    dof->setAngularLowerLimit(btVector3(0, 0, 0));
    dof->setAngularUpperLimit(btVector3(0, 0, 0));
    btVector3 lo(0, 0, 0), hi(0, 0, 0);
    switch (kind) {
      case Joint::Hinge:                              // rotation free about axis
        freeAxis(lo, hi, axis);
        dof->setAngularLowerLimit(lo);
        dof->setAngularUpperLimit(hi);
        break;
      case Joint::Slider:                             // translation free along axis
        freeAxis(lo, hi, axis);
        dof->setLinearLowerLimit(lo);
        dof->setLinearUpperLimit(hi);
        break;
      case Joint::BallSocket:                         // all rotation free
        dof->setAngularLowerLimit(btVector3(1, 1, 1));
        dof->setAngularUpperLimit(btVector3(-1, -1, -1));
        break;
      case Joint::SixDOF:                             // stays fully locked
        break;
    }
    c->m_data->constraint = dof;
    c->m_data->dof = dof;
    getDynamicsWorld()->addConstraint(dof, true);     // no collide between linked
    registerConstraint(c);
    return c;
  }

  std::shared_ptr<Constraint>
  PhysicsWorld::addSixDOF(viz3d::NodePtr a, viz3d::NodePtr b, const Vec &pivA, const Vec &pivB) {
    return makeDof(a, b, pivA, pivB, Joint::SixDOF, 0);
  }

  std::shared_ptr<Constraint>
  PhysicsWorld::addHinge(viz3d::NodePtr a, viz3d::NodePtr b, const Vec &pivA, const Vec &pivB, int axis) {
    return makeDof(a, b, pivA, pivB, Joint::Hinge, axis);
  }

  std::shared_ptr<Constraint>
  PhysicsWorld::addSlider(viz3d::NodePtr a, viz3d::NodePtr b, const Vec &pivA, const Vec &pivB, int axis) {
    return makeDof(a, b, pivA, pivB, Joint::Slider, axis);
  }

  std::shared_ptr<Constraint>
  PhysicsWorld::addBallSocket(viz3d::NodePtr a, viz3d::NodePtr b, const Vec &pivA, const Vec &pivB) {
    return makeDof(a, b, pivA, pivB, Joint::BallSocket, 0);
  }

  std::shared_ptr<SpringConstraint>
  PhysicsWorld::addSpring(viz3d::NodePtr obj, const Vec &localOffset,
                          const Vec &worldPoint, float stiffness, float damping) {
    std::shared_ptr<RigidBodyDriver> keep;
    RigidBodyDriver *drv = resolve(obj, keep);
    std::scoped_lock lock(*this);
    auto c = std::shared_ptr<SpringConstraint>(new SpringConstraint());
    c->m_data->world = this;
    c->m_data->units = getUnits();
    c->m_data->b = keep;

    // phantom static anchor the spring pulls against (the world-space attachment)
    auto *shape = new btEmptyShape();
    btRigidBody::btRigidBodyConstructionInfo ci(0, nullptr, shape);
    auto *anchor = new btRigidBody(ci);
    c->m_data->anchor = anchor;
    c->m_data->anchorShape = shape;
    getDynamicsWorld()->addRigidBody(anchor);

    auto *spring = new btGeneric6DofSpringConstraint(
        *anchor, *drv->body(), pivotFrame(c->m_data->units, worldPoint),
        pivotFrame(c->m_data->units, localOffset), true);
    spring->setLinearLowerLimit(btVector3(1, 1, 1));    // all axes free, spring-driven
    spring->setLinearUpperLimit(btVector3(0, 0, 0));
    spring->setAngularLowerLimit(btVector3(1, 1, 1));
    spring->setAngularUpperLimit(btVector3(0, 0, 0));
    for (int i = 0; i < 3; i++) spring->setStiffness(i, stiffness);
    for (int i = 0; i < 6; i++) { spring->setDamping(i, damping); spring->enableSpring(i, true); }
    // Equilibrium left at the default (DOF = 0 = frames coincident), so the
    // spring pulls obj's localOffset point toward worldPoint. (Calling
    // setEquilibriumPoint() here would freeze the *current* offset as rest.)

    c->m_data->constraint = spring;
    c->m_data->dof = spring;
    getDynamicsWorld()->addConstraint(spring, true);
    registerConstraint(c);
    return c;
  }

} // namespace icl::physics2
