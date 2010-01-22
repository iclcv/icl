#ifndef ICL_VIEW_RAY_H
#define ICL_VIEW_RAY_H

#include <ICLGeom/GeomDefs.h>
#include <ICLUtils/Exception.h>
#include <iostream>

namespace icl{
  /** \cond */
  class PlaneEquation;
  /** \endcond */

  /// This is a view-ray's line equation in parameter form
  /** A view-ray is described by the equation
      \f[ V: \mbox{offset} + \lambda \cdot \mbox{direction} \f]
  */
  struct ViewRay{
    /// Constructor with given offset and direction vector
    explicit ViewRay(const Vec &offset=Vec(), const Vec &direction=Vec());
    
    /// line offset
    Vec offset;
    
    /// line direction
    Vec direction;
    
    /// calculates line-plane intersection 
    /** @see static Camera::getIntersection function */
    Vec getIntersection(const PlaneEquation &plane) const throw (ICLException);
    
    /// evaluates ViewRay' line equation at given lambda
    Vec operator()(float lambda) const;
  };

  /// ostream operator
  std::ostream &operator<<(std::ostream &s, const ViewRay &vr);
}

#endif
