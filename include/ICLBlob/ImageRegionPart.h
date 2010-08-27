#ifndef ICL_IMAGE_REGION_PART_H
#define ICL_IMAGE_REGION_PART_H

#include <WorkingLineSegment.h>
#include <vector>
#include <list>

namespace icl{
  
  /// The ImageRegionPart represents a intermediate region part for the connected component analysis
  struct ImageRegionPart{
    
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
  
}

#endif
