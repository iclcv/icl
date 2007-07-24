#include <iclCubeObject.h>

namespace icl{
  void CubeObject::initialize(float x, float y, float z, float d){
    
    add(Vec(1,-1,1,1));
    add(Vec(1,1,1,1));//x+d2,y+d2,z+d2,1));
    add(Vec(-1,1,1,1));//x-d2,y+d2,z+d2,1));
    add(Vec(-1,-1,1,1));//x-d2,y-d2,z+d2,1));

    add(Vec(1,-1,-1,1));
    add(Vec(1,1,-1,1));//x+d2,y+d2,z+d2,1));
    add(Vec(-1,1,-1,1));//x-d2,y+d2,z+d2,1));
    add(Vec(-1,-1,-1,1));//x-d2,y-d2,z+d2,1));
    
    
    add(Tuple(0,1));
    add(Tuple(1,2));
    add(Tuple(2,3));
    add(Tuple(3,0));

    add(Tuple(4,5));
    add(Tuple(5,6));
    add(Tuple(6,7));
    add(Tuple(7,4));

    add(Tuple(0,4));
    add(Tuple(1,5));    
    add(Tuple(2,6));
    add(Tuple(3,7));
    
    setT( Mat( d/2, 0, 0, x,
               0, d/2, 0, y,
               0, 0,   1, z, 
               0, 0,   0, 1 )
          );
  }
  CubeObject::CubeObject(float x, float y, float z, float d){
    initialize(x,y,z,d);
  }
  CubeObject::CubeObject(const Vec &center, float d){
    initialize(center[0],center[1],center[2],d);
  }

}

