/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLMarkers/MarkerMetricsICL1.h                 **
** Module : ICLCV                                                **
** Authors: Christof Elbrechter                                    **
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

#pragma once

#include <ICLMarkers/MarkerCodeICL1.h>
#include <ICLUtils/Rect32f.h>
#include <ICLUtils/Point32f.h>

namespace icl{
  namespace markers{
    
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
    
  } // namespace markers
}

