// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/core/Line.h>
#include <icl/core/LineSampler.h>
#include <cmath>
#include <algorithm>
#include <icl/utils/Point32f.h>

using namespace icl::utils;

namespace icl::core {
  Line::Line(Point start, float arc, float length):
    start(start){
    end.x = start.x + static_cast<int>(cos(arc)*length);
    end.y = start.y + static_cast<int>(sin(arc)*length);
  }

  float Line::length() const{
    return ::sqrt (pow(static_cast<float>(start.x-end.x),2 ) +  pow(static_cast<float>(start.y -end.y) ,2) );
  }

  std::vector<Point> Line::sample( const Rect &limits) const{
    std::vector<Point> dst;
    LineSampler ls;
    if(limits !=  Rect::null) ls.setBoundingRect(limits);
    ls.sample(start,end,dst);
    return dst;
  }


  /// ostream operator (start-x,start-y)(end-x,end-y)
  std::ostream &operator<<(std::ostream &s, const Line &l){
    return s << l.start << l.end;
  }

  /// istream operator
  std::istream &operator>>(std::istream &s, Line &l){
    return s >> l.start >> l.end;
  }

  Point Line::findClosestPoint(const utils::Point &p) const{
    if(start == end) return start;
    Point x = p - start;
    Point32f v = Point32f(end) - Point32f(start);
    float l = v.norm();
    v *= 1./l;
    float a = (v[0]*x[0]+v[1]*x[1]);
    if(a > l) return end;
    else if(a < 0) return start;
    else return Point(Point32f(start) + v*a);
  }


  bool Line::intersects(const core::Line &other, utils::Point32f *p,
                        float *dstr, float *dsts) const{
    const Point &a = start, &b = end, &c = other.start, &d = other.end;

    const float x1 = (a.y-c.y)*(d.x-c.x)-(a.x-c.x)*(d.y-c.y);
    const float x2 = (b.x-a.x)*(d.y-c.y)-(b.y-a.y)*(d.x-c.x);
    if(x1 == 0 && x2 == 0) return false; // lines are collinear
    if(x2 == 0) return false; // lines are parallel

    const float x3 = (a.y-c.y)*(b.x-a.x)-(a.x-c.x)*(b.y-a.y);
    const float x4 = (b.x-a.x)*(d.y-c.y)-(b.y-a.y)*(d.x-c.x);
    if(x4 == 0) return false;
    const float r = x1 / x2;
    const float s = x3 / x4;

    if(dstr) *dstr = 1.0f-r;
    if(dsts) *dsts = 1.0f-s;

    if(r >= 0 && r <= 1 && s >= 0 && s <= 1){
      if(p) *p = a + (b-a)*r;
      return true;
    }
    return false;
  }

  } // namespace icl::core