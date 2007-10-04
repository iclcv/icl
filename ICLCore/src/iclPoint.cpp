#include <iclPoint.h>
#include <math.h>

namespace icl{
  const Point Point::null(0,0);

  float Point::distanceTo(const Point &p) const{
    return sqrt(pow((float) (p.x-x), 2) + pow((float) (p.y-y), 2));
  }
}
