#ifndef ICL_PRIMITIVE_H
#define ICL_PRIMITIVE_H

#include <ICLGeom/GeomDefs.h>
#include <ICLCore/Img.h>

namespace icl{
  
  struct Primitive{
    enum Type{
      vertex,line,triangle,quad,texture,nothing,PRIMITIVE_TYPE_COUNT
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
    Primitive(int a, int b, int c, int d,const Img8u &tex, bool deepCopy=false):
      a(a),b(b),c(c),d(d),tex(tex),type(texture){
        if(deepCopy) this->tex.detach();
    }

    /// not reverse ordering
    bool operator<(const Primitive &other) const{
      return z < other.z;
    }
    
    int a,b,c,d;
    GeomColor color;
    Img8u tex;
    Type type;

    
    float z;
  };
  
}

#endif
