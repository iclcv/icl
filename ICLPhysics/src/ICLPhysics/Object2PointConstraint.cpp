#include <ICLPhysics/Object2PointConstraint.h>
#include <ICLPhysics/PhysicsDefs.h>
#include <ICLPhysics/RigidBoxObject.h>
#include <BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h>
#include <BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h>

namespace icl {
  namespace physics{
    Object2PointConstraint::Object2PointConstraint(RigidObject* obj, const geom::Vec& localOffset, const geom::Vec& point, float stiffness, float damping){
      //add anchor
      anchor = new RigidBoxObject(0,0,0,0,0,0,0);
      
      //create frames and set offset
      btTransform frameA, frameB;
      frameA = btTransform::getIdentity();
      frameB = btTransform::getIdentity();
      frameA.setOrigin(icl2bullet_scaled(point));
      frameB.setOrigin(icl2bullet_scaled(localOffset));
      
      //create constraint
      btGeneric6DofSpringConstraint *cons = new btGeneric6DofSpringConstraint(*(anchor->getRigidBody()),
                        *(obj->getRigidBody()),
                        frameA,
                        frameB,
                        true);
                        
      //all axis are free
      cons->setLinearLowerLimit(btVector3(1,1,1));
      cons->setLinearUpperLimit(btVector3(0,0,0));
      cons->setAngularLowerLimit(btVector3(1,1,1));
      cons->setAngularUpperLimit(btVector3(0,0,0));
      
      //set stiffness
      cons->setStiffness(0, stiffness);
      cons->setStiffness(1, stiffness);
      cons->setStiffness(2, stiffness);
      
      //set dampening
      cons->setDamping(0, damping);
      cons->setDamping(1, damping);
      cons->setDamping(2, damping);
      cons->setDamping(3, damping);
      cons->setDamping(4, damping);
      cons->setDamping(5, damping);
      
      //enable springs
      cons->enableSpring(0,true);
      cons->enableSpring(1,true);
      cons->enableSpring(2,true);
      cons->enableSpring(3,true);
      cons->enableSpring(4,true);
      cons->enableSpring(5,true);

      cons->setEquilibriumPoint();
      
      m_constraint = cons;
      m_objects.push_back(obj);
      initUserPointer();
    }
    
    Object2PointConstraint::~Object2PointConstraint(){
      delete anchor;
    }
    
    void Object2PointConstraint::setPoint(const geom::Vec& newPoint){
      btGeneric6DofSpringConstraint* cons = dynamic_cast<btGeneric6DofSpringConstraint*>(m_constraint);
      cons->getFrameOffsetA().setOrigin(icl2bullet_scaled(newPoint));
    }
    
    void Object2PointConstraint::setLocalOffset(const geom::Vec& newLocalOffset){
      btGeneric6DofSpringConstraint* cons = dynamic_cast<btGeneric6DofSpringConstraint*>(m_constraint);
      cons->getFrameOffsetB().setOrigin(icl2bullet_scaled(newLocalOffset));
    }
    
    void Object2PointConstraint::setStiffness(float newStiffness){
      btGeneric6DofSpringConstraint* cons = dynamic_cast<btGeneric6DofSpringConstraint*>(m_constraint);
      cons->setStiffness(0, newStiffness);
      cons->setStiffness(1, newStiffness);
      cons->setStiffness(2, newStiffness);
    }
    
    void Object2PointConstraint::setDamping(float newDamping){
      btGeneric6DofSpringConstraint* cons = dynamic_cast<btGeneric6DofSpringConstraint*>(m_constraint);
      cons->setDamping(0, newDamping);
      cons->setDamping(1, newDamping);
      cons->setDamping(2, newDamping);
      cons->setDamping(3, newDamping);
      cons->setDamping(4, newDamping);
      cons->setDamping(5, newDamping);
    }
  }
}
