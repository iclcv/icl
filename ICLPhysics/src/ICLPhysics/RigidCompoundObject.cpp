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
