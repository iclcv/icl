/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLMarkers/TwoLevelRegionStructure.h           **
** Module : ICLBlob                                                **
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

#ifndef ICL_TWO_LEVEL_REGION_STRUCTURE_H
#define ICL_TWO_LEVEL_REGION_STRUCTURE_H

#include <vector>
#include <string>

#include <ICLUtils/BasicTypes.h>
#include <ICLUtils/Exception.h>

#include <ICLMarkers/RegionStructure.h>

namespace icl{
  /// Defines a common two level RegionStructure
  /** The Two level region structure is defined by
      -# the color of the root region (level 0)
      -# the sub-region counts of all sub-regions (level 1)
      
      e.g. The region structure
      <pre>
      ..........................  
      ..###############...####..  
      ..##...#####...##.........  
      ..##...#####...##...####..  
      ..##...#####...##.........  
      ..###############...####..  
      ..........................  
      </pre>
      
      Has a root region with value '.'
      and sub-region counts (0,0,0,2)\n
      The corresponding code is "b0122111"
  */
  struct TwoLevelRegionStructure : public RegionStructure{
    /// root region color
    icl8u color;
    
    /// sub-regions subregion-counts
    std::vector<int> children;
    
    /// the original code (used for comparison)
    std::string code;
    
    /// contructor from given code (see libfidtrack code)
    /** The given code is expected to be in the libfidtrack
        code style e.g. the code
        <pre>
        b01222212222122111
        </pre>
        defines a black ('b') root region ('0') whose
        - first child region (first '1') has 4 children (first '2222')
        - 2nd child region (next '1') also has 4 children (next '2222')
        - 3rd child region (3rd '1') has two children (next '22')
        - 4th, 5th and 6th child-regions have no children (ending '111')
        
        A white root region is defined by the prefix 'w0'; and the occurrence
        of the root region 0 is mandatory and therefore, it can be left out in
        the code.\n
        3- or higher level codes (which contain higher digits than 2) are not
        supported an cause an exception in the parsing process. Internally,
        the children counts are sorted for easier comparison with an actual
        region structure whose root-region is passed to the virtual match method.
    */
    TwoLevelRegionStructure(const std::string &code) throw (ICLException);
    
    /// match implementation
    virtual bool match(const ImageRegion &r) const;
    
    /// comparison operator (compares the code-strings)
    inline bool operator==(const TwoLevelRegionStructure &s) const { return code == s.code; }
  };
  
}


#endif
