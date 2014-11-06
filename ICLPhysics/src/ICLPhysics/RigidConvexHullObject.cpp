#include <ICLPhysics/RigidConvexHullObject.h>
#include <ICLPhysics/PhysicsDefs.h>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <BulletCollision/CollisionShapes/btConvexHullShape.h>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <LinearMath/btDefaultMotionState.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <ICLPhysics/MotionState.h>

namespace icl{
  namespace physics{
    RigidConvexHullObject::RigidConvexHullObject(float x, float y, float z, 
                                                               const std::vector<int> &indices, const std::vector<geom::Vec> &vertices,
                                                               geom::Vec offset,
                                                               float mass){
      btConvexHullShape *shape = new btConvexHullShape();
      for(unsigned int i = 0; i < indices.size(); i++){
        geom::Vec point = vertices[indices[i]] - offset;
        addVertex(point);
        shape->addPoint(btVector3(icl2bullet(point[0]),icl2bullet(point[1]),icl2bullet(point[2])));
      }
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
    
    RigidConvexHullObject::RigidConvexHullObject(float x, float y, float z, 
                                                               const std::vector<geom::Vec> &vertices,
                                                               geom::Vec offset,
                                                               float mass){
      std::vector<int> indices;
      indices.reserve(vertices.size());
      for(unsigned int i = 0; i < vertices.size(); i++){
        indices.push_back(i);
      }
      RigidConvexHullObject(x, y, z, indices, vertices, offset, mass);
    }
  }
}
