#ifndef ICL_MARKER_METRIC_ICL1_H
#define ICL_MARKER_METRIC_ICL1_H

#include <ICLMarkers/MarkerCodeICL1.h>
#include <ICLUtils/Rect32f.h>
#include <ICLUtils/Point32f.h>

namespace icl{
  
  /// Marker metrics for "icl1" markers
  /** each marker has
      - a border of width 1 unit -> root regions
      - 4 child regions (cr) each of height 3 units and width 11 units
      - each child regions contains 1 to 5 child-child-regions (ccr)
        each round and with diameter of 1 unit
      
      <pre>
      #############<- border (root region)
      #           # 
      # #       # #
      #         <-#-- child region cr 0
      #############
      #           # 
      # #   #   # #
      #         <-#-- child region cr 1
      #############
      #           # 
      # # #   # # #
      #         <-#-- child region cr 2
      #############
      #           # 
      # # # # # # #
      # ^        <-#-- child region cr 3
      ##|##########
        | 
        child child region ccr 0
      </pre>
      
      The ccr's are distributed towards the edges, only in case of 
      a single ccr within a cr, it is placed in the middle.
      
      The whole marker structure becomes 13 x 17 units. These are 
      mapped linearily to the actual marker size in mm;
  */
  struct MarkerMetricsICL1 : public MarkerCodeICL1 {

    /// real dimension of the root region
    Size32f root;
    
    /// child region struct
    struct CR : public Rect32f{
      /// list of child child regions
      std::vector<Rect32f> ccrs;
    };
    
    /// child regions
    CR crs[4];
    
    /// creates a metric instance from given code and real size in mm
    MarkerMetricsICL1(const MarkerCodeICL1 &c, const Size32f &markerSizeMM);
    
  };
  
}

#endif
