#include <ICLUtils/Point.h>
#include <math.h>
#include <ICLUtils/Point32f.h>
namespace icl{
  const Point Point::null(0,0);

  float Point::distanceTo(const Point &p) const{
    return sqrt(pow((float) (p.x-x), 2) + pow((float) (p.y-y), 2));
  }
  
  Point::Point(const Point32f &p){
    x = (int)::round(p.x);
    y = (int)::round(p.y);
  }

  std::ostream &operator<<(std::ostream &s, const Point &p){
    return s << "(" << p.x << ',' << p.y << ")";
  }
  
  std::istream &operator>>(std::istream &s, Point &p){
    char c;
    return s >> c >> p.x >> c >> p.y >> c;
  }

  
}
