#ifndef ICL_POINT_H
#define ICL_POINT_H
#ifdef WITH_IPP_OPTIMIZATION
#include <ipp.h>
#endif

namespace icl{
#ifndef WITH_IPP_OPTIMIZATION
  /// fallback implementation for the IppiPoint struct, defined in the ippi libb \ingroup TYPES
  struct IppiPoint {
    /// xpos
    int x;
    /// ypos
    int y;
  };
#else
#endif
  
  /// Point class of the ICL used e.g. for the Images ROI offset \ingroup TYPES
  class Point : public IppiPoint{
    public:
    /// null Point is x=0, y=0
    static const Point null;

    /// deep copy of a Point
    Point(const Point& p=null){ this->x = p.x; this->y = p.y; }

    /// create a special point
    Point(int x,int y){this->x = x;this->y = y;}

    /// checks wether the object instance is null, i.e. all elements are zero
    bool isNull() const { return (*this)==null; }

    /// checks if two points are equal
    bool operator==(const Point &s) const {return x==s.x && y==s.y;}

    /// checks if two points are not equal
    bool operator!=(const Point &s) const {return x!=s.x || y!=s.y;}

    /// adds two Points as vectors
    Point operator+(const Point &s) const {return  Point(x+s.x,y+s.y);}

    /// substracts two Points as vectors 
    Point operator-(const Point &s) const {return  Point(x-s.x,y-s.y);}

    /// scales a Points variables with a scalar value
    Point operator*(double d) const { return Point((int)(d*x),(int)(d*y));}

    /// Adds another Point inplace
    Point& operator+=(const Point &s){x+=s.x; y+=s.y; return *this;}

    /// Substacts another Point inplace
    Point& operator-=(const Point &s){x-=s.x; y-=s.y; return *this;}

    /// scales the Point inplace with a scalar
    Point& operator*=(double d) {x=(int)((double)x*d); y=(int)((double)y*d); return *this;};

    /// transforms the point by element-wise scaling
    Point transform(double xfac, double yfac) const{ 
      return Point((int)(xfac*x),(int)(yfac*y));
    }
    
    /// returns the euclidian distance to another point
    float distanceTo(const Point &p) const;
    
  };
  
} // namespace icl

#endif // ICLPOINT_H
