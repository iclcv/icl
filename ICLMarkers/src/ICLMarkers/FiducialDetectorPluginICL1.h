/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/FiducialDetectorPluginICL1.h **
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

