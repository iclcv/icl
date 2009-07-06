#include <iclPoint32f.h>
#include <math.h>
namespace icl{
  const Point32f Point32f::null(0.0,0.0);

  float Point32f::norm(float p){
    return pow( pow(x,p)+ pow(y,p), float(1)/p);
  }
  
  float Point32f::distanceTo(const Point32f &p) const{
    return sqrt(pow((float) (p.x-x), 2) + pow((float) (p.y-y), 2));
  }

  std::ostream &operator<<(std::ostream &s, const Point32f &p){
    return s << "(" << p.x << ',' << p.y << ")";
  }
  
  std::istream &operator>>(std::istream &s, Point32f &p){
    char c;
    return s >> c >> p.x >> c >> p.y >> c;
  }

}
