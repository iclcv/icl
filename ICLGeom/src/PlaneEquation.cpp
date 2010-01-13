#include <ICLGeom/PlaneEquation.h>

namespace icl{
  PlaneEquation::PlaneEquation(const Vec &offset, const Vec &normal):
    offset(offset),normal(normal){
  }

  std::ostream &operator<<(std::ostream &s, const PlaneEquation &p){
    return s << "PlaneEquation( <X- " << p.offset.transp() << "," << p.normal.transp() << "> = 0)";
  }
  
}
