#ifndef ICL_PRIMITIVE_H
#define ICL_PRIMITIVE_H

#include <iclGeomDefs.h>

namespace icl{
  
  struct Primitive{
    enum Type{
      vertex,line,triangle,quad,nothing
    };

    Primitive():
      type(nothing){
    }
    Primitive(int a, int b, const GeomColor &color=GeomColor()):
      a(a),b(b),color(color),type(line){
    }
    Primitive(int a, int b, int c, const GeomColor &color=GeomColor()):
      a(a),b(b),c(c),color(color),type(triangle){
    }
    Primitive(int a, int b, int c, int d,const GeomColor &color=GeomColor()):
      a(a),b(b),c(c),d(d),color(color),type(quad){
    }

    bool operator<(const Primitive &other) const{
      return z < other.z;
    }
    
    int a,b,c,d;
    GeomColor color;
    Type type;
    
    float z;
  };
  
}

#endif
