/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/Point.cpp                        **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter, Robert Haschke                    **
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

#include <ICLUtils/Point.h>
#include <math.h>
#include <ICLUtils/Point32f.h>

namespace icl{
  namespace utils{
    const Point Point::null(0,0);
  
    float Point::distanceTo(const Point &p) const{
      return sqrt(pow((float) (p.x-x), 2) + pow((float) (p.y-y), 2));
    }
    
    Point::Point(const Point32f &p){
      x = (int)::round(p.x);
      y = (int)::round(p.y);
    }
  
    std::ostream &operator<<(std::ostream &s, const Point &p){
      return s << "(" << p.x << ',' << p.y << ")";
    }
    
    std::istream &operator>>(std::istream &s, Point &p){
      Point32f p32;
      s >> p32;
      p = Point(p32);
      return s;
    }
  
    
  } // namespace utils
}
