/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/LensUndistortionCalibrator.h       **
** Module : ICLGeom                                                **
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

#include <ICLUtils/Point32f.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLUtils/Size.h>
#include <ICLUtils/Size32f.h>

#include <ICLIO/ImageUndistortion.h>

#include <vector>

namespace icl{

  namespace cv{
    
    class ICLGeom_API LensUndistortionCalibrator : public utils::Uncopyable{
      struct Data;
      Data *m_data;
      
      public:
      
      struct GridDefinition : public std::vector<utils::Point32f>{
        GridDefinition(const utils::Size &dims);
        GridDefinition(const utils::Size &markerGridDims, const utils::Size32f &markerSize, const utils::Size32f &markerSpacing);
        GridDefinition(const utils::Size &markerGridDims, float markerDim, float markerSpacing);
      };
      
      struct Info{
        utils::Size imageSize;
        int numPointsAdded;
        int numPointSetsAdded;
      };


      LensUndistortionCalibrator();
      
      LensUndistortionCalibrator(const utils::Size &imagesSize, const GridDefinition &gridDef);

      ~LensUndistortionCalibrator();
      
      void init(const utils::Size &imagesSize, const GridDefinition &gridDef);

      bool isNull() const;
      
      /// grid-points are predefined 
      void addPoints(const std::vector<utils::Point32f> &imagePoints);
      
      /// removes all points added before
      void clear();
      
      Info getInfo();
      
      io::ImageUndistortion computeUndistortion();
      
    };
  }
  
}
