#pragma once

#include <ICLPhysics/SliderConstraint.h>

namespace icl {
  namespace physics{
    class RigidObject;
  
    /// This constraint simulates a Hinge joint.
    class ICLPhysics_API HingeConstraint: public SliderConstraint{
      int m_rotationAxis;
      public:
      /// Creates the Hinge, with the specified frame along the rotationAxis(0 = x, 1 = y, 2 = z).
      HingeConstraint(RigidObject* a, RigidObject* b, const geom::Mat &frameInA, const geom::Mat &frameInB, int rotationAxis, const bool useLinearReferenceFrameA = true);

      HingeConstraint(RigidObject* a, RigidObject* b, const geom::Vec &pivotInA, const geom::Vec &pivotInB, int rotationAxis, const bool useLinearReferenceFrameA = true);
    };
  }
}
