#include <iclPoint32f.h>
#include <math.h>
namespace icl{
  const Point32f Point32f::null(0.0,0.0);

  float Point32f::norm(float p){
    return pow( pow(x,p)+ pow(y,p), float(1)/p);
  }

  std::ostream &operator<<(std::ostream &s, const Point32f &p){
    return s << "(" << p.x << ',' << p.y << ")";
  }
  
  std::istream &operator>>(std::istream &s, Point32f &p){
    char c;
    return s >> c >> p.x >> c >> p.y >> c;
  }

}
