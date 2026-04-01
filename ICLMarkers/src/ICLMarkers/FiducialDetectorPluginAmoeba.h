// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLMarkers/FiducialDetectorPluginHierarchical.h>

namespace icl{
  namespace markers{

    /// FiducialDetectorPlugin for reacTIVision's 'amoeba' markers\ingroup PLUGINS
    class ICLMarkers_API FiducialDetectorPluginAmoeba : public FiducialDetectorPluginHierarchical{
      /// internal data class
      class Data;

      /// hidden data pointers
      Data *data;

      /// avoid instantiation except for friends
      FiducialDetectorPluginAmoeba();
      public:
      /// this class can only be instantiated by the FiducialDetector class
      friend class FiducialDetector;

      /// Destructor
      ~FiducialDetectorPluginAmoeba();

      /// this is the only feature that is computed in a deferred way
      /** Returns the region boundary */
      virtual void getCorners2D(std::vector<utils::Point32f> &dst, FiducialImpl &impl);

      /// deferred rotation calculation
      virtual void getRotation2D(float &dst, FiducialImpl &impl);

      /// defines which features are supported
      virtual void getFeatures(Fiducial::FeatureSet &dst);

      /// defines how to find makers in the given vector of regions
      virtual void detect(std::vector<FiducialImpl*> &dst, const std::vector<cv::ImageRegion> &regions);

      /// defines how to load/remove marker definitions
      /** The Any paramter 'which' can either be a filename to a file that contains
          TwoLevelRegionStructure codes per row,
          or a newline or comma or space separated list of
          TwoLevelRegionStructure codes. The ParamList params is not used here.
      */
      virtual void addOrRemoveMarkers(bool add, const utils::Any &which, const utils::ParamList &params);
    };
  } // namespace markers
}
