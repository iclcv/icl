// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/physics/RigidCylinderObject.h>
#include <icl/physics/PhysicsDefs.h>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <BulletCollision/CollisionShapes/btCylinderShape.h>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <LinearMath/btDefaultMotionState.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <icl/physics/MotionState.h>

namespace icl::physics {
    RigidCylinderObject::RigidCylinderObject(float x, float y, float z, float r, float h, float mass){
      so = addCylinder(0, 0, 0, r, r, h, 16);

      btCylinderShape *shape = new btCylinderShapeZ(btVector3(icl2bullet(r * 0.5),icl2bullet(r * 0.5),icl2bullet(h * 0.5)));
      btTransform T;
      T.setIdentity();
      T.setOrigin(btVector3(icl2bullet(x),icl2bullet(y),icl2bullet(z)));
      MotionState* motion = new MotionState(T, static_cast<RigidObject*>(this));
      btVector3 inertia(0,0,0);
      shape->calculateLocalInertia(mass, inertia);
      btRigidBody::btRigidBodyConstructionInfo ci(mass,motion,shape,inertia);
      ci.m_linearSleepingThreshold *= METER_TO_BULLET_UNIT;
      ci.m_angularSleepingThreshold *= METER_TO_BULLET_UNIT;
      setPhysicalObject(new btRigidBody(ci));
    }
  }