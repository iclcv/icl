// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/Macros.h>
#include <ostream>

#ifdef ICL_HAVE_IPP
#include <ipp.h>
#endif

namespace icl{
  namespace utils{
  #ifndef ICL_HAVE_IPP
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
      Point operator*(double d) const { return Point(static_cast<int>(d*x),static_cast<int>(d*y));}

      /// Adds another Point inplace
      Point& operator+=(const Point &s){x+=s.x; y+=s.y; return *this;}

      /// Substacts another Point inplace
      Point& operator-=(const Point &s){x-=s.x; y-=s.y; return *this;}

      /// scales the Point inplace with a scalar
      Point& operator*=(double d) {x=static_cast<int>(static_cast<double>(x)*d); y=static_cast<int>(static_cast<double>(y)*d); return *this;};

      /// transforms the point by element-wise scaling
      Point transform(double xfac, double yfac) const;

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
