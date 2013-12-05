/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/Line32f.cpp                        **
** Module : ICLCore                                                **
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

#include <ICLCore/Line32f.h>
#include <ICLCore/Line.h>
#include <math.h>
#include <algorithm>

using namespace icl::utils;

namespace icl{
  namespace core{
  
    Line32f::Line32f(const Line &l):start(l.start),end(l.end){}
  
    Line32f::Line32f(Point32f start, float arc, float length):
      start(start){
      end.x = start.x + cos(arc)*length;
      end.y = start.y + sin(arc)*length;
    }
    
    float Line32f::length() const{
      return ::sqrt (pow( start.x-end.x,2 ) +  pow(start.y -end.y ,2) );
    }
    float Line32f::getAngle() const{
      if(start == end) return 0;
      return atan2(start.y-end.y,start.x-end.x);
    }
    Point32f Line32f::getCenter() const{
      return (start+end)*.5;
    }
    
    std::vector<Point> Line32f::sample( const Rect &limits) const{
      Point startInt = Point( (int)round(start.x), (int)round(start.y) );
      Point endInt = Point( (int)round(end.x), (int)round(end.y) );
      return Line(startInt,endInt).sample(limits);
    }
  
    /// ostream operator (start-x,start-y)(end-x,end-y)
    std::ostream &operator<<(std::ostream &s, const Line32f &l){
      return s << l.start << l.end;
    }
    
    /// istream operator
    std::istream &operator>>(std::istream &s, Line32f &l){
      return s >> l.start >> l.end;
    }

    bool Line32f::intersects(const core::Line32f &other, utils::Point32f *p,
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
  } // namespace core
}
