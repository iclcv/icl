/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLBlob module of ICL                         **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#ifndef ICL_REGION_PART_H
#define ICL_REGION_PART_H

#include <ICLBlob/ScanLine.h>
#include <vector>

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
