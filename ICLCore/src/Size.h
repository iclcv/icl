#ifndef ICL_SIZE_H
#define ICL_SIZE_H

namespace icl{
#ifndef WITH_IPP_OPTIMIZATION
  /// fallback implementation for the IppiSize struct, defined in the ippi lib
  struct IppiSize {
    // width
    int width;
    
    // height
    int height;
  };

#else
#include <ipp.h>
#endif
  
  /// Size class of the ICL
  class Size : public IppiSize{
    public:
    
    /// creates a (0,0) size
    Size(){this->width = 0;this->height = 0;}

    /// deep copy of another Size
    Size(const Size &s){ this->width = s.width;this->height = s.height; }
    
    /// creates a specified size
    Size(int width,int height){ this->width = width; this->height = height; }

    /// reutuns (width||height) as bool
    operator bool(){ return width || height; }

    /// checks if two sizes are equal
    bool operator==(const Size &s) const {return width==s.width && height==s.height;}

    /// checks if two size are not equal
    bool operator!=(const Size &s) const {return width!=s.width || height!=s.height;}

    /// add a size to another size
    Size operator+(const Size &s) const {return  Size(width+s.width,height+s.height);}

    /// substracts a size from another size
    Size operator-(const Size &s) const {return  Size(width-s.width,height-s.height);}

    /// scales the size by a scalar value
    Size operator*(double d) const { return Size((int)(d*width),(int)(d*height));}

    /// adds another size inplace
    Size& operator+=(const Size &s){width+=s.width; height+=s.height; return *this;}

    /// substracst another size inplace
    Size& operator-=(const Size &s){width-=s.width; height-=s.height; return *this;}

    /// scales the size parameters inplace by a scalar
    Size& operator*=(double d) {width=(int)((float)width*d); height=(int)((float)height*d); return *this;};
    
    /// reutrns width*height
    int getDim() const {return width*height;}
  };

}// namespace icl

#endif // ICL_SIZE_H
