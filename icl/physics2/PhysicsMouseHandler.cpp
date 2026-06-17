// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/physics2/PhysicsMouseHandler.h>
#include <icl/physics2/PhysicsWorld.h>
#include <icl/physics2/RigidBodyDriver.h>
#include <icl/physics2/Units.h>
#include <icl/geom2/Scene2.h>
#include <icl/geom2/Node.h>
#include <icl/geom/Camera.h>
#include <icl/geom/ViewRay.h>
#include <icl/geom/PlaneEquation.h>
#include <icl/qt/MouseHandler.h>

#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletDynamics/Dynamics/btDynamicsWorld.h>
#include <BulletDynamics/ConstraintSolver/btPoint2PointConstraint.h>

namespace icl::physics2 {

  struct PhysicsMouseHandler::Data {
    geom2::Scene2 *scene;
    PhysicsWorld *world;
    int camIndex;
    Units units;
    RigidBodyDriver *selected = nullptr;
    btRigidBody *grabbed = nullptr;
    btPoint2PointConstraint *constraint = nullptr;
    geom::Vec hitPoint{0, 0, 0, 1};   // ICL units, for the drag plane
  };

  PhysicsMouseHandler::PhysicsMouseHandler(int cameraIndex, geom2::Scene2 *scene,
                                           PhysicsWorld *world)
    : geom2::Scene2MouseHandler(cameraIndex, scene),
      m_data(std::make_unique<Data>()) {
    m_data->scene = scene;
    m_data->world = world;
    m_data->camIndex = cameraIndex;
    m_data->units = world->getUnits();
  }

  PhysicsMouseHandler::~PhysicsMouseHandler() { releaseGrab(); }

  void PhysicsMouseHandler::releaseGrab() {
    if (m_data->constraint) {
      auto *p2p = m_data->constraint;
      auto *body = m_data->grabbed;
      auto *world = m_data->world;
      world->enqueue([world, p2p, body]() {
        world->getDynamicsWorld()->removeConstraint(p2p);
        delete p2p;
        if (body) { body->forceActivationState(ACTIVE_TAG); body->activate(); }
      });
    }
    m_data->constraint = nullptr;
    m_data->grabbed = nullptr;
    m_data->selected = nullptr;
  }

  void PhysicsMouseHandler::process(const qt::MouseEvent &e) {
    const geom::Camera &cam = m_data->scene->getCamera(m_data->camIndex);

    const bool grabGesture =
        (e.isLeft() && e.isModifierActive(qt::ShiftModifier)) || m_data->selected;
    if (!grabGesture) { geom2::Scene2MouseHandler::process(e); return; }

    if (e.isPressEvent()) {
      geom::ViewRay ray = cam.getViewRay(e.getPos());
      geom2::Hit2 hit = m_data->scene->findObject(ray);
      if (hit.node) {
        RigidBodyDriver *d = hit.node->getDriver<RigidBodyDriver>();
        btRigidBody *body = d ? d->body() : nullptr;
        if (body && body->getInvMass() != 0.f) {   // grab dynamic bodies only
          releaseGrab();
          m_data->selected = d;
          m_data->grabbed = body;
          m_data->hitPoint = geom::Vec(hit.pos[0], hit.pos[1], hit.pos[2], 1);

          btVector3 hb = m_data->units.toBulletVec(
              Vec(hit.pos[0], hit.pos[1], hit.pos[2], 1));
          btVector3 localPivot = body->getCenterOfMassTransform().inverse() * hb;
          auto *p2p = new btPoint2PointConstraint(*body, localPivot);
          p2p->m_setting.m_impulseClamp = 30.f;
          p2p->m_setting.m_tau = 0.001f;
          m_data->constraint = p2p;

          auto *world = m_data->world;
          world->enqueue([world, p2p, body]() {
            world->getDynamicsWorld()->addConstraint(p2p, true);
            body->setActivationState(DISABLE_DEACTIVATION);
          });
        }
      }
    } else if (e.isReleaseEvent()) {
      releaseGrab();
    } else if (m_data->constraint) {
      // drag the grab target along a plane through the hit point facing the cam
      geom::Vec p = cam.estimate3DPosition(
          e.getPos(), geom::PlaneEquation(m_data->hitPoint, cam.getNorm()));
      btVector3 tb = m_data->units.toBulletVec(Vec(p[0], p[1], p[2], 1));
      auto *p2p = m_data->constraint;
      auto *body = m_data->grabbed;
      m_data->world->enqueue([p2p, tb, body]() {
        p2p->setPivotB(tb);
        if (body) body->activate();
      });
    }
  }

} // namespace icl::physics2
