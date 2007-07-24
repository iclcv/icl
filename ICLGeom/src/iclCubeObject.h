#ifndef ICL_CUBE_OBJECT_H
#define ICL_CUBE_OBJECT_H

#include <iclObject.h>

namespace icl{
  class CubeObject : public Object{
    public:
    CubeObject(float x, float y, float z, float d);
    CubeObject(const Vec &center, float d);
    private:
    void initialize(float x, float y, float z, float d);
  };
}

#endif
