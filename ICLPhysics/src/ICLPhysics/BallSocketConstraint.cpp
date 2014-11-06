#include <ICLPhysics/BallSocketConstraint.h>
#include <ICLPhysics/PhysicsDefs.h>
#include <ICLPhysics/RigidBoxObject.h>

namespace icl {
  namespace physics{
    BallSocketConstraint::BallSocketConstraint(RigidObject* a, RigidObject* b, const geom::Mat &frameInA, const geom::Mat &frameInB, const bool useLinearReferenceFrameA):
      SixDOFConstraint(a,b,frameInA,frameInB,useLinearReferenceFrameA) {
      setAngularLowerLimit(geom::Vec(1,1,1));
      setAngularUpperLimit(geom::Vec(0,0,0));
    }
    BallSocketConstraint::BallSocketConstraint(RigidObject* a, RigidObject* b, const geom::Vec &pivotInA, const geom::Vec &pivotInB, const bool useLinearReferenceFrameA):
    SixDOFConstraint(a,b,pivotInA,pivotInB,useLinearReferenceFrameA) {
      setAngularLowerLimit(geom::Vec(1,1,1));
      setAngularUpperLimit(geom::Vec(0,0,0));
    }
  }
}
