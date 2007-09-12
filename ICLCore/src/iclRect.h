#ifndef ICL_RECT_H
#define ICL_RECT_H

#include <iclPoint.h>
#include <iclSize.h>
#include <stdio.h>
#include <algorithm>
#ifdef WITH_IPP_OPTIMIZATION
#include <ipp.h>
#endif


namespace icl {
  
#ifndef WITH_IPP_OPTIMIZATION
  /// fallback implementation for the IppiRect struct, defined in the ippi lib \ingroup TYPES
  struct IppiRect {

    /// xpos of upper left corner
    int x;
    
    /// ypos of upper left corner
    int y;

    /// width
    int width;

    /// height
    int height;
  };
#else
#endif
  
  /// Rectangle class of the ICL used e.g. for the Images ROI-rect \ingroup TYPES
  /** Please take care of the following conventions when using
      Rects in the ICL library:
      <pre>
                      
                  |------ width ------>|
                ............................ 
                ............................ 
  origin(x,y)---->Xooooooooooooooooooooo....  ___
                ..oooooooooooooooooooooo....  /|\
                ..oooooooooooooooooooooo....   |
                ..oooooooooooooooooooooo.... height
                ..oooooooooooooooooooooo....   | 
                ..oooooooooooooooooooooo....  _|_
                ............................
                ............................
      </pre>
  
  */
  
  class Rect : public IppiRect{
    public:
    
    /// null Rect is w=0, h=0, x=0, y=0
    static const Rect null;
    
    /// creates a defined Rect
    Rect(int x, int y, int width, int height){
      this->x = x;
      this->y = y;
      this->width = width;
      this->height = height;
    }
    
    /// creates a new Rect with specified offset and size
    Rect(const Point &p, const Size &s){
      this->x = p.x;
      this->y = p.y;
      this->width = s.width;
      this->height = s.height;
    } 
    
    /// create a deep copy of a rect
    Rect(const Rect &r=null){
      this->x = r.x;
      this->y = r.y;
      this->width = r.width;
      this->height = r.height;
    }

    /// checks wether the object instance is null, i.e. all elements are zero
    bool isNull() const { return (*this)==null; }

    /// checks if two rects are equal
    bool operator==(const Rect &s) const {
      return x==s.x && y==s.y && width==s.width && height==s.height;
    }

    /// checks if two rects are not equal
    bool operator!=(const Rect &s) const {
      return x!=s.x || y!= s.y || width!=s.width || height!=s.height;
    }

    /// scales all parameters of the rect by a double value
    Rect operator*(double d) const {
      return Rect((int)(d*x),(int)(d*y),(int)(d*width),(int)(d*height));
    }

    /// scales all parameters of the rect by a double value
    Rect operator/(double d) const {
      return Rect((int)(x/d),(int)(y/d),(int)(width/d),(int)(height/d));
    }

    /// adds a size to the rects size
    Rect& operator+=(const Size &s){
      width+=s.width; height+=s.height; return *this;
    }
    
    /// substracs a size to the rects size
    Rect& operator-=(const Size &s){
      width-=s.width; height-=s.height; return *this;
    }
    
    /// adds a Point to the rects offset
    Rect& operator+=(const Point &p){
      x+=p.x; y+=p.y; return *this;
    }

    /// substracts a Point to the rects offset
    Rect& operator-=(const Point &p){
      x-=p.x; y-=p.y; return *this;
    }
    
    /// scales all rect params inplace
    Rect& operator*=(double d){
      x=(int)((float)x*d); 
      y=(int)((float)y*d);
      width=(int)((float)width*d); 
      height=(int)((float)height*d); 
      return *this;
    }
    /// scales all rect params inplace
    Rect& operator/=(double d){
      x=(int)((float)x/d); 
      y=(int)((float)y/d);
      width=(int)((float)width/d); 
      height=(int)((float)height/d); 
      return *this;
    }
    /// returns width*height
    int getDim() const {return width*height;}

    /// intersection of two Rects
    Rect operator&(const Rect &r) const {
       Point ul (std::max (x, r.x), std::max (y, r.y));
       Point lr (std::min (right(), r.right()), std::min (bottom(), r.bottom()));
       Rect result (ul.x, ul.y, lr.x-ul.x, lr.y-ul.y);
       if (result.width > 0 && result.height > 0) return result;
       else return null;
    }
    
    /// inplace intersection of two rects
    Rect &operator&=(const Rect &r){
      (*this)=(*this)&r;
      return *this;
    }
    
    /// union of two Rects
    Rect operator|(const Rect &r) const {
       Point ul (std::min (x, r.x), std::min (y, r.y));
       Point lr (std::max (right(), r.right()), std::max (bottom(), r.bottom()));
       return Rect (ul.x, ul.y, lr.x-ul.x, lr.y-ul.y);
    }

    /// inplace union of two rects
    Rect &operator|=(const Rect &r){
      (*this)=(*this)|r;
      return *this;
    }
    
    /// rects with negative sizes are normalized to Positive sizes
    /** e.g. the rect (5,5,-5,-5) is normalized to (0,0,5,5) */
    Rect nomalized() const {
       Rect r (*this);
       if (r.width < 0) {r.x += r.width; r.width = -r.width; }
       if (r.height < 0) {r.y += r.height; r.height = -r.height; }
       return r;
    }
    
    /// returns if a Rect containes another rect
    bool contains(const Rect &r) const {
       return x<=r.x && y <= r.y && right() >= r.right() && bottom() >= r.bottom();
    }
    
    /// returns if the Rect contains a given point
    bool contains(int x, int y) const{
      return this->x<=x && right()>=x && this->y<=y && bottom()>=y;
    }
    
    /// let the rect grow by k pixles into each direction
    /** if k<0 the rect becomes smaller
        E.g. Rect(10,10,90,90).enlarge(10) creates a Rect (0,0,100,100)
        @param k amount of pixel the rectangle is enlarged by
        @return *this
        */
    Rect &enlarge(int k){
      x-=k; y-=k; width+=2*k; height+=2*k;
      return *this;
    }
    
    /// returns an enlarged instance of this rect
    /** @see enlarge(int)*/
    Rect enlarged(int k) const{
      return Rect(*this).enlarge(k);
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
    int left() const { return x; }

    /// returns the right border position
    int right() const { return x+width; }

    /// returns the position of the bottom border
    int bottom() const { return y+height; }

    /// returns the position of the upper border 
    int top() const { return y; }

    /// returns the size of the rect
    Size size() const { return Size(width,height); }

    Rect transform(double xfac, double yfac) const { 
      return Rect((int)(xfac*x),(int)(yfac*y),(int)(xfac*width), (int)(yfac*height)); 
    }
  };

} // namespace icl

#endif // ICL_RECT_H
