#pragma once

#include <ICLPhysics/RigidObject.h>

namespace icl{
  namespace physics{
    /// Sphere with the features of a RigidObject
    class ICLPhysics_API RigidSphereObject : public RigidObject{
      protected:
      SceneObject *so;

      public:
      /// Constructor that takes the position and dimensions of the sphere, as well as the mass
      RigidSphereObject(float x, float y, float z, float r, float mass=1.0);
    };
  }
}
