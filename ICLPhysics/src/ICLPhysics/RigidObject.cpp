#include <ICLPhysics/RigidObject.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <ICLPhysics/PhysicsDefs.h>

namespace icl{
  namespace physics{
    RigidObject::RigidObject(){}  
    
    btRigidBody *RigidObject::getRigidBody() { 
      return dynamic_cast<btRigidBody*>(getCollisionObject()); 
    }

    void RigidObject::setLinearVelocity(geom::Vec velocity){
      if(!getCollisionObject()) throw utils::ICLException("RigidObject::setLinearVelocity: physical object was null");
      getRigidBody()->setLinearVelocity(scaleIcl2bullet(velocity));
    }
    
    void RigidObject::setAngularVelocity(geom::Vec velocity){
      if(!getCollisionObject()) throw utils::ICLException("RigidObject::setAngularVelocity: physical object was null");
      getRigidBody()->setAngularVelocity(icl2bullet(velocity));
    }
    
    
    geom::Vec RigidObject::getLinearVelocity(){
      if(!getCollisionObject()) throw utils::ICLException("RigidObject::getLinearVelocity: physical object was null");
      return scaleBullet2icl(getRigidBody()->getLinearVelocity());
    }
    
    geom::Vec RigidObject::getAngularVelocity(){
      if(!getCollisionObject()) throw utils::ICLException("RigidObject::getAngularVelocity: physical object was null");
      return bullet2icl(getRigidBody()->getAngularVelocity());
    }
    
    void RigidObject::applyForce(geom::Vec force, geom::Vec relPos){
      if(!getCollisionObject()) throw utils::ICLException("RigidObject::applyForce: physical object was null");
      getRigidBody()->applyForce(scaleIcl2bullet(force), scaleIcl2bullet(relPos));
    }
    
    void RigidObject::applyCentralForce(geom::Vec force){
      if(!getCollisionObject()) throw utils::ICLException("RigidObject::applyCentralForce: physical object was null");
      getRigidBody()->applyCentralForce(scaleIcl2bullet(force));
    }
    
    void RigidObject::setDamping(float linear, float angular){
      if(!getCollisionObject()) throw utils::ICLException("RigidObject::setDamping: physical object was null");
      getRigidBody()->setDamping(linear, angular);
    }
    
    RigidObject::~RigidObject(){
      delete getRigidBody()->getMotionState();
    }
    
    float RigidObject::getMass(){
      if(!getCollisionObject()) throw utils::ICLException("RigidObject::getMass: physical object was null");
      return 1.0 / getRigidBody()->getInvMass();
    }
    
    void RigidObject::setMass(float mass){
      if(!getCollisionObject()) throw utils::ICLException("RigidObject::setMass: physical object was null");
      btVector3 intertia;
      getRigidBody()->getCollisionShape()->calculateLocalInertia(mass, intertia);
      getRigidBody()->setMassProps(mass, intertia);
    }
  }
}
