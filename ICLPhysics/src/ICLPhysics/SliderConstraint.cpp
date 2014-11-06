#include <ICLPhysics/SliderConstraint.h>
#include <BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h>
#include <ICLPhysics/PhysicsDefs.h>
#include <ICLPhysics/RigidBoxObject.h>

namespace icl {
  namespace physics{
  
    void SliderConstraint::init() {
      switch(m_rotationAxis) {
      case 1:
        setAngularLowerLimit(geom::Vec(0,1,0));
        setAngularUpperLimit(geom::Vec(0,0,0));
        setLinearLowerLimit(geom::Vec(0,1,0));
        setLinearUpperLimit(geom::Vec(0,0,0));
        break;
      case 2:
        setAngularLowerLimit(geom::Vec(0,0,1));
        setAngularUpperLimit(geom::Vec(0,0,0));
        setLinearLowerLimit(geom::Vec(0,0,1));
        setLinearUpperLimit(geom::Vec(0,0,0));
        break;
      default:
        setAngularLowerLimit(geom::Vec(1,0,0));
        setAngularUpperLimit(geom::Vec(0,0,0));
        setLinearLowerLimit(geom::Vec(1,0,0));
        setLinearUpperLimit(geom::Vec(0,0,0));
        break;
      }
    }

    SliderConstraint::SliderConstraint(RigidObject* a, RigidObject* b, const geom::Mat &frameInA, const geom::Mat &frameInB, int rotationAxis, const bool useLinearReferenceFrameA):
      SixDOFConstraint(a,b,frameInA,frameInB,useLinearReferenceFrameA),
      m_rotationAxis(rotationAxis){
      init();
    }


    SliderConstraint::SliderConstraint(RigidObject* a, RigidObject* b, const geom::Vec &pivotInA, const geom::Vec &pivotInB, int rotationAxis, const bool useLinearReferenceFrameA):
      SixDOFConstraint(a,b,pivotInA,pivotInB,useLinearReferenceFrameA),
      m_rotationAxis(rotationAxis){
      init();
    }
    
    void SliderConstraint::setAngularLimits(float lower, float upper) {
      switch(m_rotationAxis) {
      case 1:
        setAngularLowerLimit(geom::Vec(0,lower,0));
        setAngularUpperLimit(geom::Vec(0,upper,0));
        break;
      case 2:
        setAngularLowerLimit(geom::Vec(0,0,lower));
        setAngularUpperLimit(geom::Vec(0,0,upper));
        break;
      default:
        setAngularLowerLimit(geom::Vec(lower,0,0));
        setAngularUpperLimit(geom::Vec(upper,0,0));
        break;
      }
    }
    
    void SliderConstraint::setLinearLimits(float lower, float upper) {
      switch(m_rotationAxis) {
      case 1:
        setLinearLowerLimit(geom::Vec(0,lower,0));
        setLinearUpperLimit(geom::Vec(0,upper,0));
        break;
      case 2:
        setLinearLowerLimit(geom::Vec(0,0,lower));
        setLinearUpperLimit(geom::Vec(0,0,upper));
        break;
      default:
        setLinearLowerLimit(geom::Vec(lower,0,0));
        setAngularUpperLimit(geom::Vec(upper,0,0));
        break;
      }
    }
  }
}
