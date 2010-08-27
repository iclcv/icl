/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLBlob/LineSegment.h                          **
** Module : ICLBlob                                                **
** Authors: Christof Elbrechter, Erik Weitnauer                    **
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

#ifndef ICL_LINE_SEGMENT_H
#define ICL_LINE_SEGMENT_H

#include <iostream>

namespace icl{
  
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
  
}
#endif
