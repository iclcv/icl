/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/LineSegment.h                          **
** Module : ICLCV                                                  **
** Authors: Christof Elbrechter, Erik Weitnauer                    **
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

#pragma once

#include <iostream>

namespace icl{
  namespace cv{
    
    /// line segment class (e.g. used for run-length coding)
    /** A line segment represents a set of successive pixels in one
        image line that have the same value. 
        Note: the LineSegment's xend location is the first pixel
              after the LineSegment, that does not have the same value
    */
    struct LineSegment{
      int x;    //!<first pixel of this line segment
      int y;    //!<y position in the image of this line segment
      int xend; //!<first pixel AFTER this line segment
  
      /// creates an empty uninitialized line segment
      LineSegment(){}
      
      /// creates a line segment with given parameters
      LineSegment(int x, int y, int xend):
        x(x),y(y),xend(xend){}
  
      /// computes the line segments length (xend-x)
      int len() const { return xend-x; }
    };
    
    
    /// ostream operator for the line-segment type
    inline std::ostream &operator<<(std::ostream &s, const LineSegment &l){
      return s << "LineSegment(x="<<l.x<<" y="<<l.y<<" len="<<l.len()<<")";
    }
    
  } // namespace cv
}
