/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/MarkerCodeICL1.h             **
** Module : ICLMarkers                                             **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/BasicTypes.h>
#include <vector>
#include <iostream>

namespace icl{
  namespace markers{

    /// Utility class for markers of type "icl1"
    /** "icl1" markers have 4 section each containing 1 to 5
        child-regions. Therefore, each marker is completely defined
        by the numbers \f$n_i\f$ of child regions. In order to avoid
        disambiguities, the sub-region counts are sorted in ascending
        order (e.g. \f$n_0 \leq n_1 \leq n2 \leq n_3 \f$. To avoid
        the degenerate case, that all \f$n_i\f$ become equal, an extra
        rule \f$ n_0 < n_3 \f$ is evaluated. A list of all valid
        codes (60) is generated by the static MarkerCodeICL1::genrate
        function.

        The extra property rootColor can be used to store the information
        about the root regions color. This can be used to create an
        extra set of makers of identical type (one with root color 0, and one
        with root color 255).

        \section ID ID computation
        The marker ID is directly computed from the set of the \f$n_i\f$.
        Since the maximum number of child child regions is 5, the base for
        ID computation is 6. We use negative marker ID's to indicate, that
        the marker structure is inverse: i.e. the markers root region is
        white
        \f[
        id(n_1,n_2,n_3,n_4) = (+1 or -1)(n_1 + 6 n_2 + 36 n_3 + 216 n_4)
        \f]
        The inverse calculation is implemented iteratively (note: the operator "/" uses integer division here)
        \f[ compute\_ni(id) = \f]
        \f[ id = abs(id) \f]
        \f[ n_4 = min(5,id/216) \f]
        \f[ id -= n_4*216 \f]
        \f[ n_3 = min(5,id/36) \f]
        \f[ id -= n_3*36; \f]
        \f[ n_2 = min(5,id/6) \f]
        \f[ id -= n_2*6 \f]
        \f[ n_1 = id; \f]

    */
    struct ICLMarkers_API MarkerCodeICL1{
      /// maximum amount of child-child-regions
      static const int P = 5;
      /// related to maximum child-child-region count base for ID computation
      static const int P1 = P+1;

      /// computed or given marker ID
      int id;

      /// computed or given set of child-child-regions
      int n[4];

        /// create instance with optionally given root color
      MarkerCodeICL1();

      /// create instance from given ID
      MarkerCodeICL1(int id);

      /// create instance from given set of child-child regions counts
      MarkerCodeICL1(const int ns[4], bool rootRegionColorIsBlack=true);

      /// get child-child region count of child-region idx
      inline int &operator[](int idx) {
        return n[idx];
      }

      /// get child-child region count of child-region idx (const)
      inline int operator[](int idx) const{
        return n[idx];
      }

      /// compares the internal id
      inline bool operator<(const MarkerCodeICL1 &t) const {
        return id < t.id;
      }

      /// computes the hamming distance of two codes
      /** Note if you only use the codes that are returned by
          the generate-function, the minimal hamming distance
          between any two markers is 2 */
      int operator-(const MarkerCodeICL1 &p) const;

      /// generates a set of 60 instances that have minimum hamming distance of 2
      static const std::vector<MarkerCodeICL1> &generate();
    };

    /// ostream operator for MarkerCodeICL1
    ICLMarkers_API std::ostream &operator<<(std::ostream &str, const MarkerCodeICL1 &c);


  } // namespace markers
}
