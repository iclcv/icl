/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/Point.h                          **
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

#include <ICLUtils/Macros.h>
#include <ostream>

#ifdef HAVE_IPP
#include <ipp.h>
#endif

namespace icl{
  namespace utils{
  #ifndef HAVE_IPP
    /// fallback implementation for the IppiPoint struct, defined in the ippi libb \ingroup TYPES
    struct IppiPoint {
      /// xpos
      int x;
      /// ypos
      int y;
    };
  #else
  #endif
  
    /** \cond */
    class Point32f;
    /** \endcond */
    
    /// Point class of the ICL used e.g. for the Images ROI offset \ingroup TYPES
    class ICLUtils_API Point : public IppiPoint{
      public:
      /// null Point is x=0, y=0
      static const Point null;

	    /// default constructor
	    Point(){ this->x = 0; this->y = 0; }

      /// deep copy of a Point
      Point(const Point& p){ this->x = p.x; this->y = p.y; }
  
      /// Create a point from given float point (values are rounded)
      Point(const Point32f &p);
      
      /// create a special point
      Point(int x,int y){this->x = x;this->y = y;}
  
      /// checks wether the object instance is null, i.e. all elements are zero
      bool isNull() const { return (*this)==null; }
  
      /// checks if two points are equal
      bool operator==(const Point &s) const {return x==s.x && y==s.y;}
  
      /// checks if two points are not equal
      bool operator!=(const Point &s) const {return x!=s.x || y!=s.y;}
  
      /// adds two Points as vectors
      Point operator+(const Point &s) const {return  Point(x+s.x,y+s.y);}
  
      /// substracts two Points as vectors 
      Point operator-(const Point &s) const {return  Point(x-s.x,y-s.y);}
  
      /// scales a Points variables with a scalar value
      Point operator*(double d) const { return Point((int)(d*x),(int)(d*y));}
  
      /// Adds another Point inplace
      Point& operator+=(const Point &s){x+=s.x; y+=s.y; return *this;}
  
      /// Substacts another Point inplace
      Point& operator-=(const Point &s){x-=s.x; y-=s.y; return *this;}
  
      /// scales the Point inplace with a scalar
      Point& operator*=(double d) {x=(int)((double)x*d); y=(int)((double)y*d); return *this;};
  
      /// transforms the point by element-wise scaling
      Point transform(double xfac, double yfac) const{ 
        return Point((int)(xfac*x),(int)(yfac*y));
      }
      
      /// returns the euclidian distance to another point
      float distanceTo(const Point &p) const;
      
      /// index based interface (returns i?y:x)
      int &operator[](int i) { return i?y:x; }
  
      /// index based interface, const (returns i?y:x)
      const int &operator[](int i) const { return i?y:x; }
    };
    
    /// ostream operator (x,y)
    ICLUtils_API std::ostream &operator<<(std::ostream &s, const Point &p);
    
    /// istream operator
    ICLUtils_API std::istream &operator>>(std::istream &s, Point &p);
  
    
  } // namespace utils
} // namespace icl

