// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
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
