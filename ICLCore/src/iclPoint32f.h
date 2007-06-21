#ifndef ICL_POINT_32F_H
#define ICL_POINT_32F_H

namespace icl{
  
  /// Single precission 3D Vectors Point class of the ICL
  class Point32f{
    public:
    
    /// x position of this point
    float x;

    /// y position of this point
    float y;
    
    /// null Point is x=0, y=0
    static const Point32f null;

    /// deep copy of a Point
    Point32f(const Point32f& p=null):x(p.x),y(p.y){}

    /// create a special point
    Point32f(float x,float y):x(x),y(y){}

    /// checks wether the object instance is null, i.e. all elements are zero
    bool isNull() const { return (*this)==null; }

    /// checks if two points are equal
    bool operator==(const Point32f &s) const {return x==s.x && y==s.y;}

    /// checks if two points are not equal
    bool operator!=(const Point32f &s) const {return x!=s.x || y!=s.y;}

    /// adds two Points as vectors
    Point32f operator+(const Point32f &s) const {return  Point32f(x+s.x,y+s.y);}

    /// substracts two Point32fs as vectors 
    Point32f operator-(const Point32f &s) const {return  Point32f(x-s.x,y-s.y);}

    /// scales a Point32fs variables with a scalar value
    Point32f operator*(double d) const { return Point32f(d*x,d*y);}

    /// Adds another Point32f inplace
    Point32f& operator+=(const Point32f &s){x+=s.x; y+=s.y; return *this;}

    /// Substacts another Point32f inplace
    Point32f& operator-=(const Point32f &s){x-=s.x; y-=s.y; return *this;}

    /// scales the Point32f inplace with a scalar
    Point32f& operator*=(double d) {x*=d; y*=d; return *this; }

    /// transforms the point by element-wise scaling
    Point32f transform(double xfac, double yfac) const{ 
      return Point32f(xfac*x,yfac*y);
    }
    
  };
  
} // namespace icl

#endif // ICLPOINT_32F_H
