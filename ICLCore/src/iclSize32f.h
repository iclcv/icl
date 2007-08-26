#ifndef ICL_SIZE_32F_H
#define ICL_SIZE_32F_H

namespace icl{
  /// Size32f class of the ICL (float valued)
  class Size32f{
    
    public:
    // width variable
    float width;
    
    // height variable
    float height;

    /// null is w=0, h=0
    static const Size32f null;
    
    /// deep copy of another Size32f
    inline Size32f(const Size32f &s=null){ this->width = s.width;this->height = s.height; }
    
    /// creates a specified size
   inline Size32f(float width,float height){ this->width = width; this->height = height; }

    /// checks wether the object instance is null, i.e. all elements are zero
    bool isNull() const { return (*this)==null; }

    /// checks if two sizes are equal
    bool operator==(const Size32f &s) const {return width==s.width && height==s.height;}

    /// checks if two size are not equal
    bool operator!=(const Size32f &s) const {return width!=s.width || height!=s.height;}

    /// add a size to another size
    Size32f operator+(const Size32f &s) const {return  Size32f(width+s.width,height+s.height);}

    /// substracts a size from another size
    Size32f operator-(const Size32f &s) const {return  Size32f(width-s.width,height-s.height);}

    /// scales the size by a scalar value
    Size32f operator*(double d) const { return Size32f(d*width,d*height);}

    /// scales the size by a scalar value
    Size32f operator/(double d) const { return (*this)*(1.0/d); }
    
    /// adds another size inplace
    Size32f& operator+=(const Size32f &s){width+=s.width; height+=s.height; return *this;}

    /// substracst another size inplace
    Size32f& operator-=(const Size32f &s){width-=s.width; height-=s.height; return *this;}

    /// scales the size parameters inplace by a scalar
    Size32f& operator*=(double d) {width*=d; height*=d; return *this; }
    
    /// scales the size parameters inplace by a scalar
    Size32f& operator/=(double d) { return (*this)*=(1.0/d); }
    
    /// reutrns width*height
    float getDim() const {return width*height;}
  };

}// namespace icl

#endif // ICL_SIZE_H
