// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Erik Weitnauer

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLCV/WorkingLineSegment.h>

#include <vector>
#include <list>

namespace icl::cv {
    /// The ImageRegionPart represents a intermediate region part for the connected component analysis
    struct ICLCV_API ImageRegionPart{

      /// internally used type for buffering children
      using children_container = std::vector<ImageRegionPart*>;

      /// internally used type for buffring segments
      using segment_container = std::vector<WorkingLineSegment*>;

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

  } // namespace icl::cv