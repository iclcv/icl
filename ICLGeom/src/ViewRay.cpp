#include <ICLGeom/ViewRay.h>
#include <ICLGeom/Camera.h>

namespace icl{
  
  ViewRay::ViewRay(const Vec &offset, const Vec &direction):
    offset(offset),direction(direction){
    this->offset[3]=this->direction[3]=1;
  }
  
  Vec ViewRay::getIntersection(const PlaneEquation &plane) const throw (ICLException){
    return Camera::getIntersection(*this,plane);
  }
  
  Vec ViewRay::operator()(float lambda) const { 
    Vec r = offset + direction*lambda; 
    r[3] = 1;
    return r;
  }

  std::ostream &operator<<(std::ostream &s, const ViewRay &vr){
    return s << "ViewRay(" << vr.offset.transp() << " + lambda * " << vr.direction.transp() << ")";
  }

}
