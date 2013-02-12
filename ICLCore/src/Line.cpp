/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/Line.cpp                                   **
** Module : ICLCore                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

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
  } // namespace core
}
