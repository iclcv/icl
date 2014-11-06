#include <ICLPhysics/RigidBoxObject.h>
#include <ICLPhysics/PhysicsDefs.h>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <LinearMath/btDefaultMotionState.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <ICLPhysics/MotionState.h>

namespace icl{
  namespace physics{
    RigidBoxObject::RigidBoxObject(float x, float y, float z, float dx, float dy, float dz, float mass){
      so = addCuboid(0,0,0,dx,dy,dz);

      btBoxShape *shape = new btBoxShape(btVector3(icl2bullet(dx/2),icl2bullet(dy/2),icl2bullet(dz/2)));
      btTransform T;
      T.setIdentity();
      T.setOrigin(btVector3(icl2bullet(x),icl2bullet(y),icl2bullet(z)));
      MotionState* motion = new MotionState(T, (RigidObject*)this);
      btVector3 inertia(0,0,0);
      shape->calculateLocalInertia(mass, inertia);
      btRigidBody::btRigidBodyConstructionInfo ci(mass,motion,shape,inertia);
      ci.m_linearSleepingThreshold *= METER_TO_BULLET_UNIT;
      ci.m_angularSleepingThreshold *= METER_TO_BULLET_UNIT;
      setPhysicalObject(new btRigidBody(ci));
    }
  }
}
