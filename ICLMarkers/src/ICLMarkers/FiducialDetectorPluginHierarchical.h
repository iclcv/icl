/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/FiducialDetectorPluginHierar **
**          chical.h                                               **
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

#include <ICLCV/ImageRegion.h>

#include <ICLMarkers/FiducialDetectorPlugin.h>
#include <ICLMarkers/RegionStructure.h>

namespace icl{
  namespace markers{
    
  
    /// Extra abstraction layer that defines a basic skeleton for the detection of hierarchical fiducials \ingroup PLUGINS
    class ICLMarkers_API FiducialDetectorPluginHierarchical : public FiducialDetectorPlugin{
      /// internal data class
      struct Data;
      
      /// internal hidden data pointer
      Data *data;
      
      protected:
      
      /// Constructor
      FiducialDetectorPluginHierarchical();
      public:
      
      /// destructor
      ~FiducialDetectorPluginHierarchical();
      
      /// defines which features are supported
      virtual void getFeatures(Fiducial::FeatureSet &dst)=0;
      
      /// defines how to detect markers from a given image
      /** In this case, regions are detected using the internal region detector.
          The regions are then passed to the other detect method */
      virtual void detect(std::vector<FiducialImpl*> &dst, const core::Img8u &image);
  
      /// defines how to find makers in the given vector of regions
      virtual void detect(std::vector<FiducialImpl*> &dst, const std::vector<cv::ImageRegion> &regions) = 0;
      
      /// defines how to load/remove marker definitions
      virtual void addOrRemoveMarkers(bool add, const utils::Any &which, const utils::ParamList &params)=0;
    };
    
  } // namespace markers
}

