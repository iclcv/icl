#ifndef ICL_DEFAULT_GROUND_OBJECT_H
#define ICL_DEFAULT_GROUND_OBJECT_H

#include <ICLPhysics/RigidBoxObject.h>

namespace icl{
  using namespace physics;
  /// Utility class, used in several demo applications
  struct DefaultGroundObject : public RigidBoxObject{
    DefaultGroundObject():RigidBoxObject(0,0,-100,1000,1000,70,0){
      setCollisionMargin(10);
      setFriction(5);
    }
  };


}


#endif
