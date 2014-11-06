#include <ICLPhysics/HingeConstraint.h>
#include <BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h>
#include <ICLPhysics/PhysicsDefs.h>
#include <ICLPhysics/RigidBoxObject.h>

namespace icl {
  namespace physics{
    HingeConstraint::HingeConstraint(RigidObject* a, RigidObject* b, const geom::Mat &frameInA, const geom::Mat &frameInB, int rotationAxis, const bool useLinearReferenceFrameA):
      SliderConstraint(a,b,frameInA,frameInB,rotationAxis,useLinearReferenceFrameA) {
      setLinearLowerLimit(geom::Vec(0,0,0));
      setLinearUpperLimit(geom::Vec(0,0,0));
    }

    HingeConstraint::HingeConstraint(RigidObject* a, RigidObject* b, const geom::Vec &pivotInA, const geom::Vec &pivotInB, int rotationAxis, const bool useLinearReferenceFrameA):
      SliderConstraint(a,b,pivotInA,pivotInB,rotationAxis,useLinearReferenceFrameA) {
      setLinearLowerLimit(geom::Vec(0,0,0));
      setLinearUpperLimit(geom::Vec(0,0,0));
    }
  }
}
