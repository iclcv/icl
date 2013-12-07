/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/ImageRegionPart.h                      **
** Module : ICLCV                                                  **
** Authors: Christof Elbrechter, Erik Weitnauer                    **
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
#pragma once

#include <ICLCV/WorkingLineSegment.h>

#include <vector>
#include <list>

namespace icl{
  namespace cv{
    
    /// The ImageRegionPart represents a intermediate region part for the connected component analysis
    struct ICL_CV_API ImageRegionPart{
      
      /// internally used type for buffering children
      typedef std::vector<ImageRegionPart*> children_container;
  
      /// internally used type for buffring segments
      typedef std::vector<WorkingLineSegment*> segment_container;
  
      /// initializes this instance with the first WorkingLoineSegment
      inline ImageRegionPart *init(WorkingLineSegment *s){
        segments.clear();
        segments.resize(1,s);
        children.clear();
        flags = 0x1; // top
        val = s->val;
        return this;
      }
      
      /// list or vector of all contained regions
      children_container children;
      
      /// list of vector of all directly contained LineSegments
      segment_container segments;
  
      /// binary flags 0b_____[collected][counted][top] 
      icl8u flags; 
  
      /// chached value
      int val;
      
      /// returns whether this ImageRegionPart is on top
      inline bool is_top() const { return flags & 0x1; }
  
      /// returns whether this ImageRegionPart has already been counted
      inline bool is_counted() const { return flags & 0x2; }
  
      /// returns whether this ImageRegionPart has already been collected
      inline bool is_collected() const { return flags & 0x4; }
  
      /// sets the counted bit to true
      inline void notify_counted() { flags |= 0x2; }
  
      /// sets the collected bit to true
      inline void notify_collected() { flags |= 0x4; }
      
      // sets the top bit to false and returns the this-pointer
      inline ImageRegionPart *adopt(){
        flags &= 0x6; 
        return this;
      }
    };
    
  } // namespace cv
}

