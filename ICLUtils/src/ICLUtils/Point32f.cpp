/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/Point32f.cpp                     **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
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

#include <ICLUtils/Point32f.h>

namespace icl{
  namespace utils{
    const Point32f Point32f::null(0.0,0.0);
  
    float Point32f::norm(float p) const{
      return pow( pow(x,p)+ pow(y,p), float(1)/p);
    }
    
    float Point32f::distanceTo(const Point32f &p) const{
      return sqrt(pow((float) (p.x-x), 2) + pow((float) (p.y-y), 2));
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
