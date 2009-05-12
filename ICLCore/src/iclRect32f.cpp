#include <iclRect32f.h>
#include <iclRect.h>



namespace icl{

  const Rect32f Rect32f::null(0,0,0,0);
  
  Rect32f::Rect32f(const Rect &rect):
    x((float)rect.x),
    y((float)rect.y),
    width((float)rect.width),
    height((float)rect.height){
  }
  
  /// ostream operator (x,y)wxy
  std::ostream &operator<<(std::ostream &s, const Rect32f &r){
    return s << r.ul() << r.getSize();
  }
  
  /// istream operator
  std::istream &operator>>(std::istream &s, Rect32f &r){
    Point32f offs;
    Size32f size;
    s >> offs;
    s >> size;
    r = Rect32f(offs,size);
    return s;
  }

  
}
