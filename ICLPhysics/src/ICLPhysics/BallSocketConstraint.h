#pragma once

#include <ICLPhysics/SixDOFConstraint.h>

namespace icl {
  namespace physics{
    class RigidObject;
  
    /// This constraint simulates a ballsocket joint.
    class ICLPhysics_API BallSocketConstraint: public SixDOFConstraint{
      public:
      BallSocketConstraint(RigidObject* a, RigidObject* b, const geom::Mat &frameInA, const geom::Mat &frameInB, const bool useLinearReferenceFrameA = true);
      BallSocketConstraint(RigidObject* a, RigidObject* b, const geom::Vec &pivotInA, const geom::Vec &pivotInB, const bool useLinearReferenceFrameA = true);
    };
  }
}
