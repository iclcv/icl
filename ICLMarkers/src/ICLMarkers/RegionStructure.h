/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/RegionStructure.h            **
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
#include <ICLUtils/SmartPtr.h>
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
      /// answers the question whether a given region matches a region structure
      /** Usually, this method is called for every region in an image. Therefore,
          a particular match-implementation should try to reject a match as fast
          as possible. E.g. by first checking whether the root region has a
          correct color value */
      virtual bool match(const cv::ImageRegion &r) const = 0;
    };

    /// Managed pointer type definition
    typedef utils::SmartPtr<RegionStructure> RegionStructurePtr;



  } // namespace markers
}

