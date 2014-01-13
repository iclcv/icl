/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/SurfFeature.h                          **
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
#include <ICLUtils/VisualizationDescription.h>
#include <utility>

namespace icl{
  namespace cv{
    
    /// Generic SURF Feature type
    struct ICL_CV_API SurfFeature{
      float x;               //!< feature x-position
      float y;               //!< feature y-position
      float scale;           //!< feature size (scale factor)
      float orientation;     //!< feature direction/orientation
      int laplacian;         //!< laplacian sign
      int clusterIndex;      //!< cluster index
      float descriptor[64];  //!< 64-Dim feature descriptor
      float dx;              //!< can be used for point motion analysis
      float dy;              //!< can be used for point motion analysis

      /// distance to other feature (squared distance of descriptors)
      float operator-(const SurfFeature &other) const;

      /// visualizes this surf feature (optionally shifted by given offsets)
      utils::VisualizationDescription vis(int dx=0, int dy=0) const;
    };
    
    /// typedef for two matching features
    typedef std::pair<SurfFeature,SurfFeature> SurfMatch;

  }
}
