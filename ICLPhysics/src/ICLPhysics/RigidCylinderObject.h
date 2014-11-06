#pragma once

#include <ICLPhysics/RigidObject.h>

namespace icl{
  namespace physics{
    /// Cylinder with the features of a RigidObject
    class ICLPhysics_API RigidCylinderObject : public RigidObject{
      protected:
      SceneObject *so;

      public:
      /// Constructor that takes the position and dimensions of the cylinder, as well as the mass
      RigidCylinderObject(float x, float y, float z, float r, float h, float mass=1.0);

    };
  }
}
