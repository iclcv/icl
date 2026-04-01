// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLMarkers/FiducialDetectorPluginHierarchical.h>

namespace icl{
  namespace markers{

    /** \cond */
    class FiducialDetector;
    /** \endcond */


    /// FiducialDetectorPlugin for ARToolkit+ like markers using BCH coded IDs \ingroup PLUGINS
    /** This is just a plugin class that is used by the FiducialDetector. Please
        refer the icl::markers::FiducialDetector class documentation for more details. */
    class ICLMarkers_API FiducialDetectorPluginICL1 : public FiducialDetectorPluginHierarchical{
      struct Data;
      Data *data;

      /// only the FiducialDetector can instantiate this class
      FiducialDetectorPluginICL1();
      public:

      /// This class cannot be used
      friend class icl::markers::FiducialDetector;

      /** \cond **/
      // for static initialization
      friend struct StaticConfigurableRegistrationFor_FiducialDetectorPluginICL1;
      /** \endcond **/

      /// Destructor
      ~FiducialDetectorPluginICL1();

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

      /// returns how to compute a list of image-position/marker-position correspondences
      /** The 2D-keypoints are the most common information that is use to compute a markers
          3D information. Each keypoint defines a 2D marker location in [mm] and a corresponding
          image location */
      virtual void getKeyPoints2D(std::vector<Fiducial::KeyPoint> &dst, FiducialImpl &impl);

      /// creates an image of a given marker
      virtual core::Img8u createMarker(const utils::Any &whichOne,const utils::Size &size, const utils::ParamList &params);


    };
  } // namespace markers
}
