/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/Size32f.h                        **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Size.h>
#include <iostream>

namespace icl{
  namespace utils{
    /// Size32f class of the ICL (float valued)
    class ICLUtils_API Size32f{

      public:
      // width variable
      float width;

      // height variable
      float height;

      /// null is w=0, h=0
      static const Size32f null;

	    /// default constructor
	    inline Size32f(){ this->width = 0; this->height = 0; }

      /// deep copy of another Size32f
      inline Size32f(const Size32f &s){ this->width = s.width;this->height = s.height; }

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
    ICLUtils_API std::ostream &operator<<(std::ostream &os, const Size32f &s);

    /// istream operator
    ICLUtils_API std::istream &operator>>(std::istream &is, Size32f &s);

  } // namespace utils
}// namespace icl

