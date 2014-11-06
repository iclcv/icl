#pragma once

#include <ICLPhysics/SixDOFConstraint.h>

namespace icl {
  namespace physics{
    /// This constraint simulates a Slider joint.
    class ICLPhysics_API SliderConstraint: public SixDOFConstraint{
      int m_rotationAxis;
      void init();
      public:
      /// Creates the Slider, with the specified frame along the rotationAxis(0 = x, 1 = y, 2 = z).
      SliderConstraint(RigidObject* a, RigidObject* b, const geom::Mat &frameInA, const geom::Mat &frameInB, int rotationAxis, const bool useLinearReferenceFrameA = true);
      /// Creates the Slider, with the specified frame along the rotationAxis(0 = x, 1 = y, 2 = z).
      SliderConstraint(RigidObject* a, RigidObject* b, const geom::Vec &pivotInA, const geom::Vec &pivotInB, int rotationAxis, const bool useLinearReferenceFrameA = true);

      /* This sets the angular limit, for the specified axis.
       * Possible ranges:
       * X-Axis: lower = -PI, upper = PI
       * Y-Axis: lower = -PI/2, upper = PI/2
       * Z-Axis: lower = -PI, upper = PI
       *
       * Setting the lower limit to be higher than the upper limit unlocks rotation on this axis.
       */
      void setAngularLimits(float lower, float upper);
      /* This sets the linear limit, for the specified axis.
       * Setting the lower limit to be higher than the upper limit unlocks translation along this Axis.
       */
      void setLinearLimits(float lower, float upper);
    };
  }
}
