// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/BasicTypes.h>
#include <ICLUtils/Exception.h>

#include <ICLMarkers/RegionStructure.h>

#include <vector>
#include <string>

namespace icl{
  namespace markers{
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
    struct ICLMarkers_API TwoLevelRegionStructure : public RegionStructure{
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
      TwoLevelRegionStructure(const std::string &code);

      /// virtual destructor
      virtual ~TwoLevelRegionStructure() = default;

      /// match implementation
      virtual bool match(const cv::ImageRegion &r) const;

      /// comparison operator (compares the code-strings)
      inline bool operator==(const TwoLevelRegionStructure &s) const { return code == s.code; }
    };

  } // namespace markers
}
