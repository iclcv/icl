#ifndef ICL_POINT_H
#define ICL_POINT_H

namespace icl{
#ifndef WITH_IPP_OPTIMIZATION
  /// fallback implementation for the IppiPoint struct, defined in the ippi libb
  struct IppiPoint {
    /// xpos
    int x;
    /// ypos
    int y;
  };
#else
#include <ipp.h>
#endif
  
  /// Point class of the ICL used e.g. for the Images ROI offset
  class Point : public IppiPoint{
    public:
    /// create a (0,0) Point 
    Point(){ this->x = 0; this->y = 0;}

    /// deep copy of a Point
    Point(const Point& p){ this->x = p.x; this->y = p.y; }

    /// create a special point
    Point(int x,int y){this->x = x;this->y = y;}

    /// returns (p.x || p.y) as bool
    operator bool() const{ return x || y ;}
    
    /// checks if two points are equal
    bool operator==(const Point &s) const {return x==s.x && y==s.y;}

    /// checks if two points are not equal
    bool operator!=(const Point &s) const {return x!=s.x || y!=s.y;}

    /// adds two Points as vectors
    Point operator+(const Point &s) const {return  Point(x+s.x,y+s.y);}

    /// substracts two Points as vectors 
    Point operator-(const Point &s) const {return  Point(x+s.x,y+s.y);}

    /// scales a Points variables with a scalar value
    Point operator*(double d) const { return Point((int)(d*x),(int)(d*y));}
    
    /// Adds another Point inplace
    Point& operator+=(const Point &s){x+=s.x; y+=s.y; return *this;}

    /// Substacts another Point inplace
    Point& operator-=(const Point &s){x-=s.x; y-=s.y; return *this;}

    // scales the Point inplace with a scalar
    Point& operator*=(double d) {x=(int)((double)x*d); y=(int)((double)y*d); return *this;};

    static const Point zero;
  };
  
} // namespace icl

#endif // ICLPOINT_H
