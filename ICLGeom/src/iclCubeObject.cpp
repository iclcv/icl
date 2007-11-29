#include <iclCubeObject.h>

namespace icl{
  void CubeObject::initialize(float x, float y, float z, float d){
    
    /*
        add(Vec(1,-1,1,1));
        add(Vec(1,1,1,1));//x+d2,y+d2,z+d2,1));
        add(Vec(-1,1,1,1));//x-d2,y+d2,z+d2,1));
        add(Vec(-1,-1,1,1));//x-d2,y-d2,z+d2,1));
        
        add(Vec(1,-1,-1,1));
        add(Vec(1,1,-1,1));//x+d2,y+d2,z+d2,1));
        add(Vec(-1,1,-1,1));//x-d2,y+d2,z+d2,1));
        add(Vec(-1,-1,-1,1));//x-d2,y-d2,z+d2,1));
    */        
    float k = d/2;
    add(Vec(x+k,y-k,z+k,1));
    add(Vec(x+k,y+k,z+k,1));
    add(Vec(x-k,y+k,z+k,1));
    add(Vec(x-k,y-k,z+k,1));

    
    add(Vec(x+k,y-k,z-k,1));
    add(Vec(x+k,y+k,z-k,1));
    add(Vec(x-k,y+k,z-k,1));
    add(Vec(x-k,y-k,z-k,1));
    
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
    
    add(Quadruple(0,1,2,3),Vec(0,100,120,155));
    add(Quadruple(4,5,6,7),Vec(0,100,140,155));
    add(Quadruple(0,3,7,4),Vec(0,100,160,155));
    add(Quadruple(1,2,6,5),Vec(0,100,180,155));
    add(Quadruple(0,1,5,4),Vec(0,100,200,155));
    add(Quadruple(3,2,6,7),Vec(0,100,220,155));
    
    /*
        setT( Mat( d/2, 0, 0, x-d/2,
        0, d/2, 0, y-d/2,
        0, 0,   1, z-d/2, 
        0, 0,   0, 1 )
        );
    */
  }
  CubeObject::CubeObject(float x, float y, float z, float d){
    initialize(x,y,z,d);
  }
  CubeObject::CubeObject(const Vec &center, float d){
    initialize(center[0],center[1],center[2],d);
  }

}

