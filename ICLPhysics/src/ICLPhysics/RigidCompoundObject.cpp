/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/RigidCompoundObject.cpp      **
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
#include <ICLPhysics/RigidCompoundObject.h>
#include <ICLPhysics/PhysicsDefs.h>
#include <BulletCollision/CollisionShapes/btCompoundShape.h>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <LinearMath/btDefaultMotionState.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <ICLPhysics/MotionState.h>

namespace icl{
  namespace physics{
    RigidCompoundObject::RigidCompoundObject(float x, float y, float z, float mass){
      btCompoundShape *shape = new btCompoundShape();
      btTransform T;
      T.setIdentity();
      T.setOrigin(btVector3(icl2bullet(x),icl2bullet(y),icl2bullet(z)));
      MotionState* motion = new MotionState(T, (RigidObject*)this);
      btVector3 inertia(1,1,1);
      btRigidBody::btRigidBodyConstructionInfo ci(mass,motion,shape,inertia);
      ci.m_linearSleepingThreshold *= METER_TO_BULLET_UNIT;
      ci.m_angularSleepingThreshold *= METER_TO_BULLET_UNIT;
      setPhysicalObject(new btRigidBody(ci));
    }

    void RigidCompoundObject::addChild(RigidObject* obj, bool passOwnership) {
      SceneObject::addChild(obj,passOwnership);
      btCompoundShape *shape = static_cast<btCompoundShape*>(getCollisionObject()->getCollisionShape());
      shape->addChildShape(icl2bullet_scaled_mat(obj->getTransformation()),obj->getCollisionObject()->getCollisionShape());
      shape->recalculateLocalAabb();
      btVector3 inertia(0,0,0);
      float mass = getMass();
      shape->calculateLocalInertia(mass, inertia);
      getRigidBody()->setMassProps(mass,inertia);
    }

    void RigidCompoundObject::removeChild(RigidObject* obj) {
      btCompoundShape *shape = static_cast<btCompoundShape*>(getCollisionObject()->getCollisionShape());
      shape->removeChildShape(obj->getCollisionObject()->getCollisionShape());
      shape->recalculateLocalAabb();
      btVector3 inertia(0,0,0);
      float mass = getMass();
      shape->calculateLocalInertia(mass, inertia);
      getRigidBody()->setMassProps(mass,inertia);
      SceneObject::removeChild(obj);
    }

    void RigidCompoundObject::removeAllChildren() {
      btCompoundShape *shape = static_cast<btCompoundShape*>(getCollisionObject()->getCollisionShape());
      while(shape->getNumChildShapes()) shape->removeChildShapeByIndex(0);
      shape->recalculateLocalAabb();
      btVector3 inertia(0,0,0);
      float mass = getMass();
      shape->calculateLocalInertia(mass, inertia);
      getRigidBody()->setMassProps(mass,inertia);
      SceneObject::removeAllChildren();
    }
  }
}
