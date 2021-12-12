/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/RegionPCAInfo.h                        **
** Module : ICLCV                                                  **
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
#include <cmath>

namespace icl{
  namespace cv{
    /// data-struct to represent local PCA information \ingroup G_RD
    class ICLCV_API RegionPCAInfo{
      public:

      /// Default Constructor
      RegionPCAInfo(float len1=0, float len2=0, float arc1=0, int cx=0, int cy=0):
      len1(len1),len2(len2),arc1(arc1),arc2(arc1+M_PI/2),cx(cx),cy(cy){}

      /// length of first major axis
      float len1;
      /// length of second major axis
      float len2;
      /// angle of the first major axis
      float arc1;
      /// angle of the second major axis (arc1+PI/2)
      float arc2;
      /// x center of the region
      int cx;
      /// y center of the region
      int cy;

      /// null PCAInfo
      static const RegionPCAInfo null;
    };
  } // namespace cv
}
