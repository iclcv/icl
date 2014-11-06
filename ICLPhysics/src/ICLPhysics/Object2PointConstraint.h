#pragma once

#include <ICLGeom/GeomDefs.h>
#include <ICLPhysics/Constraint.h>

namespace icl {
  namespace physics{
    class RigidObject;
  
    /// This constraint binds an object to a point with a spring
    class ICLPhysics_API Object2PointConstraint: public Constraint{
      RigidObject* anchor;
      public:
      /// Constructor that takes all arguments
      Object2PointConstraint(RigidObject* obj, const geom::Vec& localOffset, const geom::Vec& point, float stiffness, float damping);
      /// Destructor
      virtual ~Object2PointConstraint();
      /// Sets the Point to which the object will be bound
      void setPoint(const geom::Vec& newPoint);
      /// Sets the offset of the constraint in local coordinates of the object
      void setLocalOffset(const geom::Vec& newLocalOffset);
      /// Sets the stiffness of the joint
      void setStiffness(float newStiffness);
      /// Sets the dampening of the joint
      void setDamping(float newDamping);
    };
  }
}

