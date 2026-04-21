// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/ClippedCast.h>
#include <icl/utils/Point.h>

namespace icl::utils {
  /// Single precission 3D Vectors Point class of the ICL \ingroup TYPES
  class ICLUtils_API Point32f{
    public:

    /// x position of this point
    float x;

    /// y position of this point
    float y;

    /// null Point is x=0, y=0
    static const Point32f null;

	    /// default constructor
	    Point32f() :x(0.0f), y(0.0f){}

    /// deep copy of a Point
    Point32f(const Point32f& p):x(p.x),y(p.y){}

    /// create a special point
    Point32f(float x,float y):x(x),y(y){}

    /// craete a point by a given interger point
    Point32f(const Point &p):x(p.x),y(p.y){}

    /// checks wether the object instance is null, i.e. all elements are zero
    bool isNull() const { return (*this)==null; }

    /// checks if two points are equal
    bool operator==(const Point32f &s) const {return x==s.x && y==s.y;}

    /// checks if two points are not equal
    bool operator!=(const Point32f &s) const {return x!=s.x || y!=s.y;}

    /// adds two Points as vectors
    Point32f operator+(const Point32f &s) const {return  Point32f(x+s.x,y+s.y);}

    /// substracts two Point32fs as vectors
    Point32f operator-(const Point32f &s) const {return  Point32f(x-s.x,y-s.y);}

    /// scales a Point32fs variables with a scalar value
    Point32f operator*(double d) const { return Point32f(d*x,d*y);}

    /// Adds another Point32f inplace
    Point32f& operator+=(const Point32f &s){x+=s.x; y+=s.y; return *this;}

    /// Substacts another Point32f inplace
    Point32f& operator-=(const Point32f &s){x-=s.x; y-=s.y; return *this;}

    /// scales the Point32f inplace with a scalar
    Point32f& operator*=(double d) {x*=d; y*=d; return *this; }

    /// transforms the point by element-wise scaling
    Point32f transform(double xfac, double yfac) const;

    /// returns the euclidian distance to another point
    float distanceTo(const Point32f &p) const;

    /// Checks whether the given point p lies within the triangle defined by v1,v2 and v3
    bool inTriangle(const utils::Point32f &v1,
                    const utils::Point32f &v2,
                    const utils::Point32f &v3) const;

    /// returns the p-norm of the 2D Vector
    /** - p = 0 -> 2
        - p = 1 -> city block norm x+y
        - p = 2 -> euclidian norm
        - p->inf -> infinity norm
        @param p chooses the norm
        @return norm value
    **/
    float norm(float p=2) const;

    /// normalized this 2D vector to length=1;
    /** uses the euclidian norm!
        @ return a reference to (this) normalized
    **/
    Point32f &normalize();

    /// returns a normalized version of this Point
    /** @return normalized vec*/
    Point32f normalized() const;

    /// index based interface (returns i?y:x)
    float &operator[](int i) { return i?y:x; }

    /// index based interface, const (returns i?y:x)
    const float &operator[](int i) const { return i?y:x; }
  };

  /// ostream operator (x,y)
  ICLUtils_API std::ostream &operator<<(std::ostream &s, const Point32f &p);

  /// istream operator
  ICLUtils_API std::istream &operator>>(std::istream &s, Point32f &p);


  } // namespace icl::utils