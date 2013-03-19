/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/Line.cpp                           **
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

#include <ICLCore/Line.h>
#include <ICLCore/LineSampler.h>
#include <math.h>
#include <algorithm>
#include <ICLUtils/Point32f.h>

using namespace icl::utils;

namespace icl{
  namespace core{
    
  
    Line::Line(Point start, float arc, float length):
      start(start){
      end.x = start.x + (int)(cos(arc)*length);
      end.y = start.y + (int)(sin(arc)*length);
    }
    
    float Line::length() const{
      return ::sqrt (pow((float) (start.x-end.x),2 ) +  pow((float) (start.y -end.y) ,2) );
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

  } // namespace core
}
