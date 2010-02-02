#include <ICLUtils/Rect.h>
#include <ICLUtils/Rect32f.h>
#include <cmath>
const icl::Rect icl::Rect::null(0,0,0,0);

namespace icl{
  /// ostream operator (x,y)wxy
  std::ostream &operator<<(std::ostream &s, const Rect &r){
    return s << r.ul() << r.getSize();
  }
  
  /// istream operator
  std::istream &operator>>(std::istream &s, Rect &r){
    Point offs;
    Size size;
    s >> offs >> size;
    r = Rect(offs,size);
    return s;
  }
  Rect::Rect(const Rect32f &other){
    x = round(other.x);
    y = round(other.y);
    width = round(other.width);
    height = round(other.height);
  }
}
