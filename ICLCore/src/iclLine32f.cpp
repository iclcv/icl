#include "iclLine32f.h"
#include <math.h>
#include <algorithm>

using namespace std;

namespace icl{

  Line32f::Line32f(Point32f start, float arc, float length):
    start(start){
    end.x = start.x + cos(arc)*length;
    end.y = start.y + sin(arc)*length;
  }
  
  float Line32f::length() const{
    return ::sqrt (pow( start.x-end.x,2 ) +  pow(start.y -end.y ,2) );
  }
  
  std::vector<Point> Line32f::sample( const Rect &limits) const{
    Point startInt = Point( (int)round(start.x), (int)round(start.y) );
    Point endInt = Point( (int)round(end.x), (int)round(end.y) );
    return Line(startInt,endInt).sample(limits);
  }
  void Line32f::sample(vector<int> &xs, vector<int> &ys, const Rect &limits ) const{
    Point startInt = Point( (int)round(start.x), (int)round(start.y) );
    Point endInt = Point( (int)round(end.x), (int)round(end.y) );
    return Line(startInt,endInt).sample(xs,ys,limits);
  }

  /// ostream operator (start-x,start-y)(end-x,end-y)
  std::ostream &operator<<(std::ostream &s, const Line32f &l){
    return s << l.start << l.end;
  }
  
  /// istream operator
  std::istream &operator>>(std::istream &s, Line32f &l){
    return s >> l.start >> l.end;
  }
}
