#ifndef ICL_REGION_PART_H
#define ICL_REGION_PART_H

#include <iclScanLine.h>

namespace icl{
  
  /// Low-Level Utility class for RegionDetector
  /** The region detection algorithm (implemented in the RegionDetector
      class) internally creates a tree structure of so called
      RegionPart's. The "Part"-Nomenclature was chosen, the stress, that
      underlying regions may be constructed of several parts (a parent Part
      and a tree of childs and child-childs and so on).\n
      Each RegionPart contains a list of ScanLines as well as a list of child
      RegionParts. In addition top level parent parts are marked by a
      a flag to accelerate Region construction procedure.
   */
  class RegionPart{
    public:
    
    /// Create a new Top-Level RegionPart (top is set to true)
    RegionPart():top(true){}

    /// add a ScanLine to this part
    inline void add(const ScanLine &l){
      scanlines.push_back(l);
    }
    
    /// add a child part to this part (p's top level flag is set to false)
    inline void add(RegionPart *p){
      parts.push_back(p);
      p->top = false;
    }

    /// contained list of ScanLines
    std::vector<ScanLine> scanlines;
    
    /// list of child parts
    std::vector<RegionPart*> parts;
    
    /// top-level flag
    bool top;
    
    /// internally used utility function
    static void del_func(RegionPart *p) { delete p; }
  };
}
#endif
