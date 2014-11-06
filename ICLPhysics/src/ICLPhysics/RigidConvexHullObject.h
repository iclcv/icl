#pragma once

#include <ICLPhysics/RigidObject.h>

namespace icl{
  namespace physics{
    /// ConvexHullObject witht he features of a RigidObject that can be created from a pointcloud
    class ICLPhysics_API RigidConvexHullObject : public RigidObject{
      public:
      /// Constructor that takes the Position and a subset of vertices specified by the index list, as well as the mass
      RigidConvexHullObject(float x, float y, float z, 
                                   const std::vector<int> &indices, const std::vector<geom::Vec> &vertices, 
                                   geom::Vec offset = geom::Vec(0,0,0,0),
                                   float mass=1.0);
                                   
                                   
      /// Constructor that takes the Position and a list of vertices, as well as the mass
      RigidConvexHullObject(float x, float y, float z, 
                                   const std::vector<geom::Vec> &vertices, 
                                   geom::Vec offset = geom::Vec(0,0,0,0),
                                   float mass=1.0);
    };
  }
}
