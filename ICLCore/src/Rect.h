#ifndef ICL_RECT_H
#define ICL_RECT_H

namespace icl{
#ifndef WITH_IPP_OPTIMIZATION
  struct IppiRect {int x,y,width,height;};
#else
#include <ipp.h>
#endif
  
  /// Rectangle class of the ICL used e.g. for the Images ROI-rect
  class Rect : public IppiRect{
    public:
    Rect(){
      this->x = 0;
      this->y = 0;
      this->width = 0;
      this->height = 0;
    }
    Rect(int x, int y, int width, int height){
      this->x = x;
      this->y = y;
      this->width = width;
      this->height = height;
    }
    Rect(const Point &p, const Size &s){
      this->x = p.x;
      this->y = p.y;
      this->width = s.width;
      this->height = s.height;
    } 
    Rect(const Rect &r){
      this->x = r.x;
      this->y = r.y;
      this->width = r.width;
      this->height = r.height;
    }
    operator bool() const{
      return x || y || width || height;
    }

    bool operator==(const Rect &s) const {
      return x==s.x && y==s.y && width==s.width && height==s.height;
    }
    bool operator!=(const Rect &s) const {
      return x!=s.x || y!= s.y || width!=s.width || height!=s.height;
    }
    Rect operator+(const Rect &s) const {
      return  Rect(x+s.x,y+s.y,width+s.width,height+s.height);
    }
    Rect operator-(const Rect &s) const {
      return  Rect(x-s.x,y-s.y,width-s.width,height-s.height);
    }
    Rect operator*(double d) const {
      return Rect((int)(d*x),(int)(d*y),(int)(d*width),(int)(d*height));
    }
    Rect& operator+=(const Rect &s){
      x+=s.x; y+=s.y; width+=s.width; height+=s.height; return *this;
    }
    Rect& operator-=(const Rect &s){
      x-=s.x; y-=s.y; width-=s.width; height-=s.height; return *this;
    }
    Rect& operator*=(double d){
      x=(int)((float)x*d); 
      y=(int)((float)y*d);
      width=(int)((float)width*d); 
      height=(int)((float)height*d); 
      return *this;
    };
    
    int getDim() const {return width*height;}

    Rect operator&(const Rect &r) const {
      printf("ERROR!!! Rect::intersection operator & is not yet implemented \n");
      return Rect();
    }
    Rect operator|(const Rect &r) const {
      printf("ERROR!!! Rect::union operaotr | is not yet implemented \n");
      return Rect();
    }
    Rect nomalized(){
      printf("ERROR!!! Rect::normalized is not yet implemented \n");
      return Rect();
    }
    bool contains(const Rect &r){
      printf("ERROR!!! Rect::contains is not yet implemented \n");
      return Rect();
    }
  };

} // namespace icl

#endif // ICL_RECT_H
