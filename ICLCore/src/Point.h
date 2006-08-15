#ifndef ICL_POINT_H
#define ICL_POINT_H

namespace icl{
#ifndef WITH_IPP_OPTIMIZATION
  struct IppiPoint {int x,y;};
#else
#include <ipp.h>
#endif
  
  /// Point class of the ICL used e.g. for the Images ROI offset
  class Point : public IppiPoint{
    public:
    Point(){ this->x = 0; this->y = 0;}
    Point(const Point& p){ this->x = p.x; this->y = p.y; }
    Point(int x,int y){this->x = x;this->y = y;}

    operator bool() const{ return x || y ;}
    
    bool operator==(const Point &s) const {return x==s.x && y==s.y;}
    bool operator!=(const Point &s) const {return x!=s.x || y!=s.y;}
    Point operator+(const Point &s) const {return  Point(x+s.x,y+s.y);}
    Point operator-(const Point &s) const {return  Point(x+s.x,y+s.y);}
    Point operator*(int i) const{ return Point(x*i,y*i);};
    Point operator*(float f) const { return Point((int)(f*x),(int)(f*y));}
    Point& operator+=(const Point &s){x+=s.x; y+=s.y; return *this;}
    Point& operator-=(const Point &s){x-=s.x; y-=s.y; return *this;}
    Point& operator*=(int i) {x*=i; y*=i; return *this;};
    Point& operator*=(float f) {x=(int)((float)x*f); y=(int)((float)y*f); return *this;};
  };
  
} // namespace icl

#endif // ICLPOINT_H
