#ifndef ICL_SIZE_H
#define ICL_SIZE_H

namespace icl{
#ifndef WITH_IPP_OPTIMIZATION
  struct IppiSize {int width,height;};
#else
#include <ipp.h>
#endif
  
  /// Size class of the ICL
  class Size : public IppiSize{
    public:
    Size(){this->width = 0;this->height = 0;}
    Size(const Size &s){ this->width = s.width;this->height = s.height; }
    Size(int width,int height){ this->width = width; this->height = height; }

    operator bool(){ return width || height; }

    bool operator==(const Size &s) const {return width==s.width && height==s.height;}
    bool operator!=(const Size &s) const {return width!=s.width || height!=s.height;}
    Size operator+(const Size &s) const {return  Size(width+s.width,height+s.height);}
    Size operator-(const Size &s) const {return  Size(width-s.width,height-s.height);}
    Size operator*(double d) const { return Size((int)(d*width),(int)(d*height));}
    Size& operator+=(const Size &s){width+=s.width; height+=s.height; return *this;}
    Size& operator-=(const Size &s){width-=s.width; height-=s.height; return *this;}
    Size& operator*=(double d) {width=(int)((float)width*d); height=(int)((float)height*d); return *this;};
    
    int getDim() const {return width*height;}
  };

}// namespace icl

#endif // ICL_SIZE_H
