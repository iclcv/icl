/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/SixDOFConstraint.cpp         **
** Module : ICLPhysics                                             **
** Author : Christof Elbrechter, Matthias Esau                     **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/
#include <ICLPhysics/SixDOFConstraint.h>
#include <BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h>
#include <ICLPhysics/PhysicsDefs.h>
#include <ICLPhysics/RigidBoxObject.h>

namespace icl {
  namespace physics{
  void SixDOFConstraint::init(RigidObject* a, RigidObject* b, const geom::Mat &frameInA, const geom::Mat &frameInB, const bool useLinearReferenceFrameA) {
      btGeneric6DofConstraint *cons = new btGeneric6DofConstraint(*a->getRigidBody(),
                                                                  *b->getRigidBody(),
                                                                  icl2bullet_scaled_mat(frameInA),
                                                                  icl2bullet_scaled_mat(frameInB),
                                                                  useLinearReferenceFrameA);

      m_constraint = cons;
      //per default all axes are locked
      setLinearLowerLimit(geom::Vec(0,0,0));
      setLinearUpperLimit(geom::Vec(0,0,0));
      setAngularLowerLimit(geom::Vec(0,0,0));
      setAngularUpperLimit(geom::Vec(0,0,0));
      m_objects.push_back(a);
      m_objects.push_back(b);
			initUserPointer();
    }
    SixDOFConstraint::SixDOFConstraint(RigidObject* a, RigidObject* b, const geom::Mat &frameInA, const geom::Mat &frameInB,const bool useLinearReferenceFrameA) {
      init(a,b,frameInA,frameInB,useLinearReferenceFrameA);
    }
    SixDOFConstraint::SixDOFConstraint(RigidObject* a, RigidObject* b, const geom::Vec &pivotInA, const geom::Vec &pivotInB,bool useLinearReferenceFrameA) {
    init(a, b,
         geom::Mat(1,0,0,pivotInA[0],
                   0,1,0,pivotInA[1],
                   0,0,1,pivotInA[2],
                   0,0,0,1),
         geom::Mat(1,0,0,pivotInB[0],
                   0,1,0,pivotInB[1],
                   0,0,1,pivotInB[2],
                   0,0,0,1),
      useLinearReferenceFrameA);
    }
    SixDOFConstraint::SixDOFConstraint(RigidObject* a, RigidObject* b, bool useLinearReferenceFrameA) {
    init(a, b,
      geom::Mat(1,0,0,0,
          0,1,0,0,
          0,0,1,0,
          0,0,0,1),
      geom::Mat(1,0,0,0,
          0,1,0,0,
          0,0,1,0,
          0,0,0,1),
      useLinearReferenceFrameA);
    }

    void SixDOFConstraint::setFrames(const geom::Mat &frameA, const geom::Mat &frameB) {
      dynamic_cast<btGeneric6DofConstraint*>(m_constraint)->setFrames(icl2bullet_scaled_mat(frameA),icl2bullet_scaled_mat(frameB));
    }

    void SixDOFConstraint::setPivot(const geom::Vec &pivotInA, const geom::Vec &pivotInB) {
      setFrames(geom::Mat(1,0,0,pivotInA[0],
                          0,1,0,pivotInA[1],
                          0,0,1,pivotInA[2],
                          0,0,0,1),
                geom::Mat(1,0,0,pivotInB[0],
                          0,1,0,pivotInB[1],
                          0,0,1,pivotInB[2],
                          0,0,0,1));
      }

    void SixDOFConstraint::setLinearLowerLimit(const geom::Vec &lower) {
      dynamic_cast<btGeneric6DofConstraint*>(m_constraint)->setLinearLowerLimit(icl2bullet_scaled(lower));
    }
    void SixDOFConstraint::setLinearUpperLimit(const geom::Vec &upper) {
      dynamic_cast<btGeneric6DofConstraint*>(m_constraint)->setLinearUpperLimit(icl2bullet_scaled(upper));
    }
    geom::Vec SixDOFConstraint::getLinearLowerLimit() {
      btVector3 lower;
      dynamic_cast<btGeneric6DofConstraint*>(m_constraint)->getLinearLowerLimit(lower);
      return bullet2icl_scaled(lower);
    }
    geom::Vec SixDOFConstraint::getLinearUpperLimit() {
      btVector3 upper;
      dynamic_cast<btGeneric6DofConstraint*>(m_constraint)->getLinearUpperLimit(upper);
      return bullet2icl_scaled(upper);
    }

		float SixDOFConstraint::getAngle(int index) {
			return dynamic_cast<btGeneric6DofConstraint*>(m_constraint)->getAngle(index);
		}

    void SixDOFConstraint::setAngularLowerLimit(const geom::Vec &lower) {
      dynamic_cast<btGeneric6DofConstraint*>(m_constraint)->setAngularLowerLimit(icl2bullet_unscaled(lower));
    }
    void SixDOFConstraint::setAngularUpperLimit(const geom::Vec &upper) {
      dynamic_cast<btGeneric6DofConstraint*>(m_constraint)->setAngularUpperLimit(icl2bullet_unscaled(upper));
    }
    geom::Vec SixDOFConstraint::getAngularLowerLimit() {
      btVector3 lower;
      dynamic_cast<btGeneric6DofConstraint*>(m_constraint)->getAngularLowerLimit(lower);
      return bullet2icl_unscaled(lower);
    }
    geom::Vec SixDOFConstraint::getAngularUpperLimit() {
      btVector3 upper;
      dynamic_cast<btGeneric6DofConstraint*>(m_constraint)->getAngularUpperLimit(upper);
      return bullet2icl_unscaled(upper);
    }

		void SixDOFConstraint::setAngularMotor(int index, bool enableMotor, float targetVelocity, float maxMotorForce, bool force_activation) {
      btGeneric6DofConstraint *cons = dynamic_cast<btGeneric6DofConstraint*>(m_constraint);
      cons->getRotationalLimitMotor(index)->m_enableMotor = enableMotor;
      cons->getRotationalLimitMotor(index)->m_targetVelocity = targetVelocity;
			cons->getRotationalLimitMotor(index)->m_maxMotorForce = icl2bullet(icl2bullet(maxMotorForce));
			if (force_activation && m_objects.size() > 1) {
				m_objects[0]->setActive(true);
				m_objects[1]->setActive(true);
			}
    }

		void SixDOFConstraint::setLinearMotor(int index, bool enableMotor, float targetVelocity, float maxMotorForce, bool force_activation) {
      btGeneric6DofConstraint *cons = dynamic_cast<btGeneric6DofConstraint*>(m_constraint);
      cons->getTranslationalLimitMotor()->m_enableMotor[index] = enableMotor;
      cons->getTranslationalLimitMotor()->m_targetVelocity[index] = icl2bullet(targetVelocity);
			cons->getTranslationalLimitMotor()->m_maxMotorForce[index] = icl2bullet(maxMotorForce);
			if (force_activation && m_objects.size() > 1) {
				m_objects[0]->setActive(true);
				m_objects[1]->setActive(true);
			}
    }
  }
}
