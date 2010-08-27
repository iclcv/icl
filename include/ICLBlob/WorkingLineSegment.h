/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLBlob/WorkingLineSegment.h                   **
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

#ifndef ICL_WORKING_LINE_SEGMENT_H
#define ICL_WORKING_LINE_SEGMENT_H

#include <ICLBlob/LineSegment.h>

namespace icl{
  
  /** \cond */
  struct ImageRegionData;
  struct ImageRegionPart;
  /** \endcond */
  
  /// The working line segment class extends the LineSegment class by some working parameters
  /** These extra parameters are not used in the ImageRegion class. Here, we can use C++'s 
      slicing assignment to cut the extra information using std::copy */
  struct WorkingLineSegment : public LineSegment{
    /// image value
    int val; 
   
    /// additional payload that is used internally
    union{
      void *anyData;
      ImageRegionPart *reg;
      ImageRegionData *ird;
      int regID;
    };
    
    /// Constructor
    WorkingLineSegment():reg(0){}
    
    /// intialization function
    inline void init(int x, int y, int xend, int val){
      this->x = x;
      this->y = y;
      this->xend = xend;
      this->val = val;
    }
    
    /// reset function (sets payload to NULL)
    inline void reset() {
      reg = 0;
    }
  };

  /// Overloaded ostream-operator for WorkingLineSegment-instances
  inline std::ostream &operator<<(std::ostream &str,const WorkingLineSegment &ls){
    return str << (const LineSegment&)ls << '[' << ls.regID  << ']';
    
  }
  
  
}
#endif
