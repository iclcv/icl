#include <ICLPhysics/SixDOFConstraint.h>
#include <BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h>
#include <ICLPhysics/PhysicsDefs.h>
#include <ICLPhysics/RigidBoxObject.h>

namespace icl {
  namespace physics{
  void SixDOFConstraint::init(RigidObject* a, RigidObject* b, const geom::Mat &frameInA, const geom::Mat &frameInB, const bool useLinearReferenceFrameA) {
      btGeneric6DofConstraint *cons = new btGeneric6DofConstraint(*a->getRigidBody(),
                                                                  *b->getRigidBody(),
                                                                  icl2bullet(frameInA),
                                                                  icl2bullet(frameInB),
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
      dynamic_cast<btGeneric6DofConstraint*>(m_constraint)->setFrames(icl2bullet(frameA),icl2bullet(frameB));
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
      dynamic_cast<btGeneric6DofConstraint*>(m_constraint)->setLinearLowerLimit(scaleIcl2bullet(lower));
    }
    void SixDOFConstraint::setLinearUpperLimit(const geom::Vec &upper) {
      dynamic_cast<btGeneric6DofConstraint*>(m_constraint)->setLinearUpperLimit(scaleIcl2bullet(upper));
    }
    geom::Vec SixDOFConstraint::getLinearLowerLimit() {
      btVector3 lower;
      dynamic_cast<btGeneric6DofConstraint*>(m_constraint)->getLinearLowerLimit(lower);
      return scaleBullet2icl(lower);
    }
    geom::Vec SixDOFConstraint::getLinearUpperLimit() {
      btVector3 upper;
      dynamic_cast<btGeneric6DofConstraint*>(m_constraint)->getLinearUpperLimit(upper);
      return scaleBullet2icl(upper);
    }
    
    
    void SixDOFConstraint::setAngularLowerLimit(const geom::Vec &lower) {
      dynamic_cast<btGeneric6DofConstraint*>(m_constraint)->setAngularLowerLimit(icl2bullet(lower));
    }
    void SixDOFConstraint::setAngularUpperLimit(const geom::Vec &upper) {
      dynamic_cast<btGeneric6DofConstraint*>(m_constraint)->setAngularUpperLimit(icl2bullet(upper));
    }
    geom::Vec SixDOFConstraint::getAngularLowerLimit() {
      btVector3 lower;
      dynamic_cast<btGeneric6DofConstraint*>(m_constraint)->getAngularLowerLimit(lower);
      return bullet2icl(lower);
    }
    geom::Vec SixDOFConstraint::getAngularUpperLimit() {
      btVector3 upper;
      dynamic_cast<btGeneric6DofConstraint*>(m_constraint)->getAngularUpperLimit(upper);
      return bullet2icl(upper);
    }
      
    void SixDOFConstraint::setAngularMotor(int index, bool enableMotor, float targetVelocity, float maxMotorForce) {
      btGeneric6DofConstraint *cons = dynamic_cast<btGeneric6DofConstraint*>(m_constraint);
      cons->getRotationalLimitMotor(index)->m_enableMotor = enableMotor;
      cons->getRotationalLimitMotor(index)->m_targetVelocity = targetVelocity;
      cons->getRotationalLimitMotor(index)->m_maxMotorForce = icl2bullet(icl2bullet(maxMotorForce));
    }
    
    void SixDOFConstraint::setLinearMotor(int index, bool enableMotor, float targetVelocity, float maxMotorForce) {
      btGeneric6DofConstraint *cons = dynamic_cast<btGeneric6DofConstraint*>(m_constraint);
      cons->getTranslationalLimitMotor()->m_enableMotor[index] = enableMotor;
      cons->getTranslationalLimitMotor()->m_targetVelocity[index] = icl2bullet(targetVelocity);
      cons->getTranslationalLimitMotor()->m_maxMotorForce[index] = icl2bullet(maxMotorForce);
    }
  }
}
