#ifndef ICL_RECT_H
#define ICL_RECT_H

#include "Point.h"
#include "Size.h"
#include <stdio.h>

namespace icl{

  
#ifndef WITH_IPP_OPTIMIZATION
  /// fallback implementation for the IppiRect struct, defined in the ippi lib
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
#include <ipp.h>
#endif
  
  /// Rectangle class of the ICL used e.g. for the Images ROI-rect
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
    /// returns (x || y || width || height) as bool
    operator bool() const{
      return x || y || width || height;
    }

    /// returns !(bool)(*this)
    bool operator!() const { return !(bool)(*this); } 
    
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
    };
    
    /// returns width*height
    int getDim() const {return width*height;}

    /// intersection of two Rects (NOT IMPLEMENTED)
    Rect operator&(const Rect &r) const {
      (void)r;
      printf("ERROR!!! Rect::intersection operator & is not yet implemented \n");
      return Rect();
    }
    
    /// union of two Rects (NOT IMPLEMENTED)
    Rect operator|(const Rect &r) const {
      (void)r;
      printf("ERROR!!! Rect::union operaotr | is not yet implemented \n");
      return Rect();
    }
    
    /// rects with negative sizes are normalized to Positive sizes (NOT IMPLEMENTED)
    /** e.g. the rect (5,5,-5,-5) is normalized to (0,0,5,5) */
    Rect nomalized(){
      printf("ERROR!!! Rect::normalized is not yet implemented \n");
      return Rect();
    }
    
    /// returns if a Rect containes another rect (NOT IMPLEMENTED)
    bool contains(const Rect &r){
      (void)r;
      return x<=r.x && y <= r.y && right() >= r.right() && bottom() >= r.bottom();
    }
    
    bool contains(int x, int y){
      return this->x<=x && right()>=x && this->y<=y && bottom()>=y;
    }

    /// returns lower left point of the rect
    Point ul() const {
      return Point(x,y);
    }
    /// returns upper left point of the rect
    Point ll() const {
      return Point(x,y+height);
    }
    /// returns lower right point of the rect
    Point ur() const {
      return Point(x+width,y);
    }
    /// returns upper right point of the rect
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
  };

} // namespace icl

#endif // ICL_RECT_H
