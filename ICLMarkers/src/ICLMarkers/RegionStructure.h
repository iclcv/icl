// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <memory>
#include <string>

namespace icl{
  ///@cond
  namespace cv{ struct ImageRegion; }
  ///@endcond


  namespace markers{

    /// region structure interface class
    /** A region structure can be defined arbitrarily, It defines
        how a single image region is matched agains a given structure
        instance */
    struct RegionStructure{
      virtual ~RegionStructure() = default;
      /// answers the question whether a given region matches a region structure
      /** Usually, this method is called for every region in an image. Therefore,
          a particular match-implementation should try to reject a match as fast
          as possible. E.g. by first checking whether the root region has a
          correct color value */
      virtual bool match(const cv::ImageRegion &r) const = 0;
    };

    /// Managed pointer type definition
    using RegionStructurePtr = std::shared_ptr<RegionStructure>;



  } // namespace markers
}
