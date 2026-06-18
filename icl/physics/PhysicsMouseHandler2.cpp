// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/physics/PhysicsMouseHandler2.h>
#include <icl/physics/PhysicsWorld.h>
#include <icl/physics/RigidObject.h>
#include <icl/physics/Object2PointConstraint.h>
#include <icl/geom2/Scene2.h>
#include <icl/geom/Camera.h>
#include <icl/geom/ViewRay.h>

namespace icl::physics {

  PhysicsMouseHandler2::PhysicsMouseHandler2(int cameraIndex, geom2::Scene2 *scene,
                                             PhysicsWorld *world)
      : qt::MouseHandler(),
        m_scene(scene), m_world(world), m_camIndex(cameraIndex),
        m_selected(nullptr), m_constraint(nullptr) {}

  PhysicsMouseHandler2::~PhysicsMouseHandler2() { removeConstraint(); }

  void PhysicsMouseHandler2::removeConstraint() {
    if (m_constraint) {
      m_world->removeConstraint(m_constraint);
      delete m_constraint;
      m_constraint = nullptr;
    }
  }

  qt::MouseResult PhysicsMouseHandler2::process(const qt::MouseEvent &e) {
    using qt::MouseResult;
    const geom::Camera &cam = m_scene->getCamera(m_camIndex);

    // Shift+Left enters / stays in grab mode; otherwise forward to the camera
    // handler installed after this one.
    if ((e.isLeft() && e.isModifierActive(qt::ShiftModifier)) || m_selected) {
      if (e.isPressEvent()) {
        geom::ViewRay ray = cam.getViewRay(e.getPos());
        geom::Vec normal;
        PhysicsObject *obj = nullptr;
        if (m_world->rayCast(ray, 10000.f, obj, normal, m_hitPoint)) {
          m_selected = dynamic_cast<RigidObject *>(obj);
          if (m_selected) {
            if (m_selected->hasContactResponse()) {
              geom::Mat invTrans = m_selected->getTransformation().inv();
              geom::Vec relPos = invTrans * m_hitPoint;
              removeConstraint();
              m_constraint = new Object2PointConstraint(m_selected, relPos,
                                                        m_hitPoint, 500.f, 0.002f);
              m_world->addConstraint(m_constraint);
            } else {
              m_selected = nullptr;
            }
          }
        }
      } else if (e.isReleaseEvent()) {
        if (m_selected) {
          m_selected = nullptr;
          removeConstraint();
        }
      } else if (m_selected) {
        geom::Vec p = cam.estimate3DPosition(
            e.getPos(), geom::PlaneEquation(m_hitPoint, cam.getNorm()));
        m_constraint->setPoint(p);
        m_selected->activate();
      }
      return MouseResult::Processed;
    }
    return MouseResult::Forward;
  }

} // namespace icl::physics
