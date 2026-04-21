// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/markers/MarkerCodeICL1.h>
#include <icl/utils/Rect32f.h>
#include <icl/utils/Point32f.h>

namespace icl::markers {
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
  struct ICLMarkers_API MarkerMetricsICL1 : public MarkerCodeICL1 {

    /// real dimension of the root region
    utils::Size32f root;

    /// child region struct
    struct CR : public utils::Rect32f{
      /// list of child child regions
      std::vector<utils::Rect32f> ccrs;
    };

    /// child regions
    CR crs[4];

    /// creates a metric instance from given code and real size in mm
    MarkerMetricsICL1(const MarkerCodeICL1 &c, const utils::Size32f &markerSizeMM);

  };

  } // namespace icl::markers