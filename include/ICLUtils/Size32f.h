/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLUtils module of ICL                        **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
** University of Bielefeld                                                **
** Contact: nivision@techfak.uni-bielefeld.de                             **
**                                                                        **
** This file is part of the ICLUtils module of ICL                        **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#ifndef ICL_SIZE_32F_H
#define ICL_SIZE_32F_H

#include <iostream>
#include <ICLUtils/Size.h>

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

    /// creates a float-size from given int size
    inline Size32f(const Size &other):width(other.width),height(other.height){}
    
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

  /// ostream operator WIDTHxHEIGHT
  std::ostream &operator<<(std::ostream &os, const Size32f &s);
  
  /// istream operator
  std::istream &operator>>(std::istream &is, Size32f &s);

}// namespace icl

#endif // ICL_SIZE_H
