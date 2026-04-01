// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLUtils/Point32f.h>

namespace icl{
  namespace utils{
    const Point32f Point32f::null(0.0,0.0);

    float Point32f::norm(float p) const{
      return pow( pow(x,p)+ pow(y,p), float(1)/p);
    }

    Point32f Point32f::transform(double xfac, double yfac) const {
      return Point32f(xfac * x, yfac * y);
    }

    Point32f &Point32f::normalize(){
      float l = norm(); x /= l; y /= l; return *this;
    }

    Point32f Point32f::normalized() const {
      return Point32f(*this).normalize();
    }

    float Point32f::distanceTo(const Point32f &p) const{
      return sqrt(pow(static_cast<float>(p.x-x), 2) + pow(static_cast<float>(p.y-y), 2));
    }

    std::ostream &operator<<(std::ostream &s, const Point32f &p){
      return s << "(" << p.x << ',' << p.y << ")";
    }


    std::istream &operator>>(std::istream &s, Point32f &p){
      char c,d;
      s >> c;
      if ( ((c >= '0') && (c <= '9')) || c=='-' ){
        s.unget();
      }
      s >> p.x;
      s >> d; // anything delimiting ...
      s >> p.y;
      if (!( ((c >= '0') && (c <= '9')) || c=='-' )){
        s >> d;
        if(c == '|' && d != '|') s.unget();
        if(c == '(' && d != ')') s.unget();
        if(c == '[' && d != ']') s.unget();
        if(c == '{' && d != '}') s.unget();
      }
      return s;
    }

    inline float point_in_triangle_sign(const Point32f &p1, const Point32f &p2, const Point32f &p3){
      return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
    }

    bool Point32f::inTriangle(const Point32f &v1, const Point32f &v2, const Point32f &v3) const{
      bool b1 = point_in_triangle_sign(*this, v1, v2) <= 0;
      bool b2 = point_in_triangle_sign(*this, v2, v3) <= 0;
      bool b3 = point_in_triangle_sign(*this, v3, v1) <= 0;
      return (b1 == b2) && (b2 == b3);
    }

  } // namespace utils
}
