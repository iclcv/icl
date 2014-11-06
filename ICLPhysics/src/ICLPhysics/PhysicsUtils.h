#pragma once

#include <ICLGeom/Geom.h>

namespace icl{
  namespace geom{
    class Scene;
  }
  namespace physics{
  
    class PhysicsWorld;
    class PhysicsObject;
    
    /// removes all rigid object from the given scene and world that are below minZ
    void remove_fallen_objects(geom::Scene *scene, PhysicsWorld *world, float minZ = -2000);

    /// adds num random objects
    void add_clutter(geom::Scene *scene, PhysicsWorld *world, int num = 30);

    /// Calculates the distance between the 2 closest points of the objects a and b
    void calcDistance(PhysicsObject* a, PhysicsObject* b, geom::Vec &pointOnA, geom::Vec &pointOnB, float &distance);

    /// Calculates the distance between the 2 closest points of the objects a and b
    void calcDistance(PhysicsObject* a, PhysicsObject* b, geom::Mat transA, geom::Mat transB, geom::Vec &pointOnA, geom::Vec &pointOnB, float &distance);
    
    /// Creates primitives from blobs. The last blob is assumed to be the Table the objects rest on.
    void createObjectsFromBlobs(const std::vector< std::vector<int> > &indices, const std::vector<geom::Vec> &vertices, std::vector<PhysicsObject*> &objects);
  }
}
