#ifndef ICL_SIZE_H
#define ICL_SIZE_H

namespace icl {
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
    /// null is w=0, h=0
    static const Size null;
    
    /// deep copy of another Size
    inline Size(const Size &s=null){ this->width = s.width;this->height = s.height; }
    
    /// creates a specified size
   inline Size(int width,int height){ this->width = width; this->height = height; }

    /// checks wether the object instance is null, i.e. all elements are zero
    bool isNull() const { return (*this)==null; }

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

    /// scales the size by a scalar value
    Size operator/(double d) const { return (*this)*(1.0/d); }
    
    /// adds another size inplace
    Size& operator+=(const Size &s){width+=s.width; height+=s.height; return *this;}

    /// substracst another size inplace
    Size& operator-=(const Size &s){width-=s.width; height-=s.height; return *this;}

    /// scales the size parameters inplace by a scalar
    Size& operator*=(double d) {width=(int)((float)width*d); height=(int)((float)height*d); return *this;};

    /// scales the size parameters inplace by a scalar
    Size& operator/=(double d) { return (*this)*=(1.0/d); }
    
    /// reutrns width*height
    int getDim() const {return width*height;}
  };

}// namespace icl

#endif // ICL_SIZE_H
