#include <iclRect.h>

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

}
