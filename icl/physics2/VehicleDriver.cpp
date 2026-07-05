// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/physics2/VehicleDriver.h>
#include <icl/physics2/PhysicsWorld.h>
#include <icl/physics2/StateBuffer.h>
#include <icl/physics2/CollisionShapeFactory.h>
#include <icl/viz3d/Node.h>
#include <icl/viz3d/CylinderNode.h>
#include <icl/viz3d/Material.h>
#include <icl/cv3d/GeomDefs.h>
#include <icl/utils/Macros.h>

#include <btBulletDynamicsCommon.h>
#include <BulletDynamics/Vehicle/btRaycastVehicle.h>
#include <BulletDynamics/Vehicle/btVehicleRaycaster.h>

#include <array>
#include <cmath>

namespace icl::physics2 {

  // wheel order: 0=FL, 1=FR (front, steered), 2=RL, 3=RR (rear, driven)
  enum { FL, FR, RL, RR, NUM_WHEELS };

  struct VehicleDriver::Data {
    PhysicsWorld &world;
    Config cfg;
    Units units;

    btCollisionShape *shape = nullptr;
    btRigidBody *chassis = nullptr;
    btDefaultVehicleRaycaster *raycaster = nullptr;
    btRaycastVehicle *vehicle = nullptr;
    bool added = false;

    StateSlot chassisSlot;
    std::array<StateSlot, NUM_WHEELS> wheelSlots;
    std::vector<viz3d::NodePtr> wheelNodes;
    Mat wheelAlign = Mat::id();   // local mesh orientation (cylinder axis -> axle)

    Data(PhysicsWorld &w, const Config &c) : world(w), cfg(c) {}
  };

  VehicleDriver::VehicleDriver(PhysicsWorld &world, const Config &cfg)
    : m_data(std::make_unique<Data>(world, cfg)) {}

  VehicleDriver::~VehicleDriver() {
    // onDetach() normally ran; guard against leaks if not.
    delete m_data->vehicle;
    delete m_data->raycaster;
    delete m_data->chassis;
    deleteShape(m_data->shape);
  }

  void VehicleDriver::onAttach() {
    auto *n = node();
    if (!n) return;
    m_data->units = m_data->world.getUnits();
    const Units &u = m_data->units;
    const Config &cfg = m_data->cfg;

    // --- chassis rigid body (shape from the host node) ---
    m_data->shape = shapeFromNode(n, u);
    if (!m_data->shape) {
      ERROR_LOG("VehicleDriver: host node carries no usable geometry for a chassis shape");
      return;
    }
    btTransform T = u.toBullet(n->getTransformation(true));
    btVector3 inertia(0, 0, 0);
    m_data->shape->calculateLocalInertia(cfg.chassisMass, inertia);
    btRigidBody::btRigidBodyConstructionInfo ci(cfg.chassisMass, nullptr, m_data->shape, inertia);
    ci.m_startWorldTransform = T;
    m_data->chassis = new btRigidBody(ci);
    m_data->chassis->setUserPointer(static_cast<viz3d::Driver *>(this));
    m_data->chassis->setActivationState(DISABLE_DEACTIVATION);  // a parked car must still respond
    m_data->chassis->setWorldTransform(T);
    m_data->world.addBody(m_data->chassis);

    // Scale the (SI-calibrated) suspension constants to this world's gravity.
    // physics2 scales lengths by iclToBullet but leaves time/mass alone, so the
    // Bullet-side gravity is g_si*1000*iclToBullet (= 10x at the default 0.01).
    // The standard vehicle stiffness 20 / maxForce 6000 assume g_si — without
    // this the suspension is ~10x too soft and the car bottoms out on its box.
    const float gBullet = 9810.f * u.iclToBullet;          // world gravity, Bullet units
    const float gScale = gBullet / 9.81f;                  // vs SI
    const float stiffness = cfg.suspensionStiffness * gScale;
    const float dampRelax = cfg.suspensionDamping * std::sqrt(gScale);
    const float dampComp = cfg.suspensionCompression * std::sqrt(gScale);
    const float maxSuspForce = cfg.chassisMass * gBullet;  // hold the full weight

    // --- raycast vehicle ---
    btRaycastVehicle::btVehicleTuning tuning;
    tuning.m_suspensionStiffness = stiffness;
    tuning.m_suspensionCompression = dampComp;
    tuning.m_suspensionDamping = dampRelax;
    tuning.m_frictionSlip = cfg.wheelFriction;
    tuning.m_maxSuspensionForce = maxSuspForce;
    m_data->raycaster = new btDefaultVehicleRaycaster(m_data->world.getDynamicsWorld());
    m_data->vehicle = new btRaycastVehicle(tuning, m_data->chassis, m_data->raycaster);
    m_data->chassis->setActivationState(DISABLE_DEACTIVATION);
    m_data->vehicle->setCoordinateSystem(0, 2, 1);   // right=X, up=Z, forward=Y (Z-up)
    m_data->world.addAction(m_data->vehicle);

    // wheel connection points from the chassis AABB (Bullet units)
    btTransform id; id.setIdentity();
    btVector3 aabbMin, aabbMax;
    m_data->shape->getAabb(id, aabbMin, aabbMax);
    const btVector3 half = (aabbMax - aabbMin) * 0.5f;
    const float zc = aabbMin.z() + u.toBullet(cfg.connectionHeight);   // chassis bottom
    const float radius = u.toBullet(cfg.wheelRadius);
    const float rest = u.toBullet(cfg.suspensionRestLength);
    const btVector3 dir(0, 0, -1);    // suspension points down
    const btVector3 axle(1, 0, 0);    // wheel spin axis (left-right); sign sets +force -> +Y
    const float tx = half.x() * cfg.trackFrac, ty = half.y() * cfg.baseFrac;
    const btVector3 conn[NUM_WHEELS] = {
      btVector3(-tx, +ty, zc),  // FL
      btVector3(+tx, +ty, zc),  // FR
      btVector3(-tx, -ty, zc),  // RL
      btVector3(+tx, -ty, zc),  // RR
    };
    for (int i = 0; i < NUM_WHEELS; i++) {
      bool front = (i == FL || i == FR);
      btWheelInfo &w = m_data->vehicle->addWheel(conn[i], dir, axle, rest, radius, tuning, front);
      w.m_suspensionStiffness = stiffness;
      w.m_wheelsDampingRelaxation = dampRelax;
      w.m_wheelsDampingCompression = dampComp;
      w.m_maxSuspensionForce = maxSuspForce;
      w.m_frictionSlip = cfg.wheelFriction;
      w.m_rollInfluence = cfg.rollInfluence;
    }

    // --- visual wheel nodes (the scene renders these) ---
    // CylinderNode's axis is local Z; the wheel spins about its axle (local X in
    // the wheel transform), so rotate Z->X (about Y by 90deg) or the disc renders
    // edge-on (invisible). Applied per-frame as worldTransform * wheelAlign.
    m_data->wheelAlign = Mat::id();
    m_data->wheelAlign(0,0) = 0; m_data->wheelAlign(0,2) = 1;
    m_data->wheelAlign(2,0) = -1; m_data->wheelAlign(2,2) = 0;
    auto tyre = viz3d::Material::fromColor(cv3d::GeomColor(30,30,30,255));
    m_data->wheelNodes.clear();
    for (int i = 0; i < NUM_WHEELS; i++) {
      auto wheel = viz3d::CylinderNode::create(0, 0, 0, cfg.wheelRadius*2, cfg.wheelRadius*2,
                                               cfg.wheelWidth, 20);
      wheel->setMaterial(tyre);
      m_data->wheelNodes.push_back(std::static_pointer_cast<viz3d::Node>(wheel));
    }

    // --- post-step capture: snapshot chassis + wheel transforms (sim thread) ---
    m_data->world.addCapture(this, [this]() {
      m_data->chassisSlot.publish(m_data->chassis->getWorldTransform());
      for (int i = 0; i < NUM_WHEELS; i++) {
        m_data->vehicle->updateWheelTransform(i, true);
        m_data->wheelSlots[i].publish(m_data->vehicle->getWheelTransformWS(i));
      }
    });

    m_data->chassisSlot.publish(T);   // seed render side
    m_data->added = true;
  }

  void VehicleDriver::onDetach() {
    if (m_data->added) {
      m_data->world.removeCapture(this);
      m_data->world.removeAction(m_data->vehicle);
      m_data->world.removeBody(m_data->chassis);
      m_data->added = false;
    }
    delete m_data->vehicle;   m_data->vehicle = nullptr;
    delete m_data->raycaster; m_data->raycaster = nullptr;
    delete m_data->chassis;   m_data->chassis = nullptr;
    deleteShape(m_data->shape);     m_data->shape = nullptr;
  }

  void VehicleDriver::sync(double /*dt*/, double alpha) {
    if (auto *n = node())
      n->setTransformation(m_data->units.toIcl(m_data->chassisSlot.sample((float)alpha)));
    for (int i = 0; i < (int)m_data->wheelNodes.size(); i++) {
      m_data->wheelNodes[i]->setTransformation(
          m_data->units.toIcl(m_data->wheelSlots[i].sample((float)alpha)) * m_data->wheelAlign);
    }
  }

  // --- controls: enqueue so they apply on the sim thread before the next step
  //     (engine force is read during the vehicle's updateAction tick) ---

  void VehicleDriver::setEngineForce(float force) {
    m_data->world.enqueue([this, force]() {
      if (!m_data->vehicle) return;
      m_data->vehicle->applyEngineForce(force, RL);   // rear-wheel drive
      m_data->vehicle->applyEngineForce(force, RR);
    });
  }

  void VehicleDriver::setBrake(float brake) {
    m_data->world.enqueue([this, brake]() {
      if (!m_data->vehicle) return;
      for (int i = 0; i < NUM_WHEELS; i++) m_data->vehicle->setBrake(brake, i);
    });
  }

  void VehicleDriver::setSteering(float radians) {
    m_data->world.enqueue([this, radians]() {
      if (!m_data->vehicle) return;
      m_data->vehicle->setSteeringValue(radians, FL);
      m_data->vehicle->setSteeringValue(radians, FR);
    });
  }

  void VehicleDriver::reset(const Mat &pose) {
    const btTransform T = m_data->units.toBullet(pose);
    m_data->world.enqueue([this, T]() {
      if (!m_data->chassis || !m_data->vehicle) return;
      m_data->chassis->setWorldTransform(T);
      m_data->chassis->setLinearVelocity(btVector3(0, 0, 0));
      m_data->chassis->setAngularVelocity(btVector3(0, 0, 0));
      m_data->chassis->clearForces();
      for (int i = 0; i < NUM_WHEELS; i++) {
        m_data->vehicle->applyEngineForce(0.f, i);
        m_data->vehicle->setBrake(0.f, i);
        m_data->vehicle->setSteeringValue(0.f, i);
        m_data->vehicle->updateWheelTransform(i, true);
      }
      m_data->chassis->activate(true);
      m_data->chassisSlot.publish(T);   // seed the render side immediately
    });
  }

  void VehicleDriver::setCcd(float motionThreshold, float sweptSphereRadius) {
    if (!m_data->chassis) return;
    std::scoped_lock lock(m_data->world);
    m_data->chassis->setCcdMotionThreshold(m_data->units.toBullet(motionThreshold));
    m_data->chassis->setCcdSweptSphereRadius(m_data->units.toBullet(sweptSphereRadius));
  }

  float VehicleDriver::getSpeedKmh() const {
    if (!m_data->vehicle) return 0.f;
    // getCurrentSpeedKmHour treats Bullet units as metres; correct for our scale.
    const float metersPerUnit = 0.001f / m_data->units.iclToBullet;
    return m_data->vehicle->getCurrentSpeedKmHour() * metersPerUnit;
  }

  Mat VehicleDriver::getChassisPose() const {
    return m_data->units.toIcl(m_data->chassisSlot.sample(1.0f));
  }

  const std::vector<viz3d::NodePtr> &VehicleDriver::getWheelNodes() const {
    return m_data->wheelNodes;
  }

} // namespace icl::physics2
