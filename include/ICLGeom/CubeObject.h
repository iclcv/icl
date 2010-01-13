#ifndef ICL_CUBE_OBJECT_H
#define ICL_CUBE_OBJECT_H

#include <ICLGeom/Object.h>

namespace icl{
  
  /// implementation of a unity cube object
  class CubeObject : public Object{
    public:
    /// create a new cube object at position x,y,z with edge-length d
    CubeObject(float x, float y, float z, float d);

    /// create a new cube object at position "center" with edge-length d
    CubeObject(const Vec &center, float d);

    private:
    /// internal initilization function called by the constructors
    void initialize(float x, float y, float z, float d);
  };
}

#endif
