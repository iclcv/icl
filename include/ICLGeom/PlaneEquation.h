#ifndef ICL_PLANE_EQUATION_H
#define ICL_PLANE_EQUATION_H

#include <ICLGeom/GeomDefs.h>
#include <iostream>

namespace icl{
  
  /// Utility structure for calculation of view-ray / plane intersections
  struct PlaneEquation{

    /// Constructor with given offset and direction vector
    explicit PlaneEquation(const Vec &offset=Vec(), const Vec &normal=Vec());
    
      /// line offset
    Vec offset;
    
    /// line direction
    Vec normal;
  };

  /// ostream operator
  std::ostream &operator<<(std::ostream &s, const PlaneEquation &p);
}

#endif
