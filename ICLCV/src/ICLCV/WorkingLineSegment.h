// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Erik Weitnauer

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLCV/LineSegment.h>

namespace icl::cv {
    /** \cond */
    struct ImageRegionData;
    struct ImageRegionPart;
    /** \endcond */

    /// The working line segment class extends the LineSegment class by some working parameters
    /** These extra parameters are not used in the ImageRegion class. Here, we can use C++'s
        slicing assignment to cut the extra information using std::copy */
    struct ICLCV_API WorkingLineSegment : public LineSegment{
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


  } // namespace icl::cv