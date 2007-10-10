#ifndef ICL_RECT_32F_H
#define ICL_RECT_32F_H

#include <iclPoint32f.h>
#include <iclSize32f.h>
#include <stdio.h>
#include <algorithm>

namespace icl {
  
  /// Floatin point precision implementation of the Rect class \ingroup TYPES
  class Rect32f{
    public:
   
    float x;      ///!< x pos (upper left)
    float y;      ///!< y pos (upper left)
    float width;  ///!< width of the rect
    float height; ///!< height of the rect

    /// static null instance (0,0,0,0)
    static const Rect32f null;
    
    /// creates a defined Rect32f
    Rect32f(float x, float y, float width, float height):
    x(x),y(y),width(width),height(height){
      this->x = x;
      this->y = y;
      this->width = width;
      this->height = height;
    }
    
    /// creates a new Rect32f with specified offset and size
    Rect32f(const Point32f &p, const Size32f &s){
      this->x = p.x;
      this->y = p.y;
      this->width = s.width;
      this->height = s.height;
    } 
    
    /// create a deep copy of a rect
    Rect32f(const Rect32f &r=null){
      this->x = r.x;
      this->y = r.y;
      this->width = r.width;
      this->height = r.height;
    }

    /// checks wether the object instance is null, i.e. all elements are zero
    bool isNull() const { return (*this)==null; }

    /// checks if two rects are equal
    bool operator==(const Rect32f &s) const {
      return x==s.x && y==s.y && width==s.width && height==s.height;
    }

    /// checks if two rects are not equal
    bool operator!=(const Rect32f &s) const {
      return x!=s.x || y!= s.y || width!=s.width || height!=s.height;
    }

    /// scales all parameters of the rect by a double value
    Rect32f operator*(double d) const {
      return Rect32f(d*x,d*y,d*width,d*height);
    }

    /// scales all parameters of the rect by a double value
    Rect32f operator/(double d) const {
      return Rect32f(d/x,d/y,d/width,d/height);
    }

    /// adds a size to the rects size
    Rect32f& operator+=(const Size32f &s){
      width+=s.width; height+=s.height; return *this;
    }
    
    /// substracs a size to the rects size
    Rect32f& operator-=(const Size32f &s){
      width-=s.width; height-=s.height; return *this;
    }
    
    /// adds a Point to the rects offset
    Rect32f& operator+=(const Point32f &p){
      x+=p.x; y+=p.y; return *this;
    }

    /// substracts a Point to the rects offset
    Rect32f& operator-=(const Point32f &p){
      x-=p.x; y-=p.y; return *this;
    }
    
    /// scales all rect params inplace
    Rect32f& operator*=(double d){
      x*=d;
      y*=d;
      width*=d;
      height*=d;
      return *this;
    }
    /// scales all rect params inplace
    Rect32f& operator/=(double d){
      x/=d;
      y/=d;
      width/=d;
      height/=d;
      return *this;
    }
    /// returns width*height
    float getDim() const {return width*height;}

    /// intersection of two Rect32fs
    Rect32f operator&(const Rect32f &r) const {
       Point ul (iclMax (x, r.x), iclMax (y, r.y));
       Point lr (iclMin (right(), r.right()), iclMin (bottom(), r.bottom()));
       Rect32f result (ul.x, ul.y, lr.x-ul.x, lr.y-ul.y);
       if (result.width > 0 && result.height > 0) return result;
       else return null;
    }
    
    /// inplace intersection of two rects
    Rect32f &operator&=(const Rect32f &r){
      (*this)=(*this)&r;
      return *this;
    }
    
    /// union of two Rect32fs
    Rect32f operator|(const Rect32f &r) const {
       Point ul (iclMin (x, r.x), iclMin (y, r.y));
       Point lr (iclMax (right(), r.right()), iclMax (bottom(), r.bottom()));
       return Rect32f (ul.x, ul.y, lr.x-ul.x, lr.y-ul.y);
    }

    /// inplace union of two rects
    Rect32f &operator|=(const Rect32f &r){
      (*this)=(*this)|r;
      return *this;
    }
    
    /// rects with negative sizes are normalized to Positive sizes
    /** e.g. the rect (5,5,-5,-5) is normalized to (0,0,5,5) */
    Rect32f nomalized() const {
       Rect32f r (*this);
       if (r.width < 0) {r.x += r.width; r.width = -r.width; }
       if (r.height < 0) {r.y += r.height; r.height = -r.height; }
       return r;
    }
    
    /// returns if a Rect32f containes another rect
    bool contains(const Rect32f &r) const {
       return x<=r.x && y <= r.y && right() >= r.right() && bottom() >= r.bottom();
    }
    
    /// returns if the Rect32f contains a given point
    bool contains(float x, float y) const{
      return this->x<=x && right()>=x && this->y<=y && bottom()>=y;
    }
    
    /// let the rect grow by k pixles into each direction
    /** if k<0 the rect becomes smaller
        E.g. Rect32f(10,10,90,90).enlarge(10) creates a Rect32f (0,0,100,100)
        @param k amount of pixel the rectangle is enlarged by
        @return *this
        */
    Rect32f &enlarge(float k){
      x-=k; y-=k; width+=2*k; height+=2*k;
      return *this;
    }
    
    /// returns an enlarged instance of this rect
    /** @see enlarge(float)*/
    Rect32f enlarged(float k) const{
      return Rect32f(*this).enlarge(k);
    }
    
    
    /// returns upper left point of the rect
    Point ul() const {
      return Point(x,y);
    }
    /// returns lower left point of the rect
    Point ll() const {
      return Point(x,y+height);
    }
    /// returns upper right point of the rect
    Point ur() const {
      return Point(x+width,y);
    }
    /// returns lower right point of the rect
    Point lr() const {
      return Point(x+width,y+height);
    }

    /// returns the left border position
    float left() const { return x; }

    /// returns the right border position
    float right() const { return x+width; }

    /// returns the position of the bottom border
    float bottom() const { return y+height; }

    /// returns the position of the upper border 
    float top() const { return y; }

    /// returns the size of the rect
    Size32f size() const { return Size32f(width,height); }

    Rect32f transform(double xfac, double yfac) const { 
      return Rect32f(x*xfac,y*yfac,width*xfac,height*yfac);
    }
  };

} // namespace icl

#endif // ICL_RECT_H
