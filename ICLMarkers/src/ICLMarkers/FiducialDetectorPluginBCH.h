// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLMarkers/FiducialDetectorPluginForQuads.h>

namespace icl::markers {
    /** \cond */
    class FiducialDetector;
    /** \endcond */


    /// FiducialDetectorPlugin for ARToolkit+ like markers using BCH coded IDs \ingroup PLUGINS
    /** This is just a plugin class that is used by the FiducialDetector. Please
        refer the icl::markers::FiducialDetector class documentation for more details. */
    class ICLMarkers_API FiducialDetectorPluginBCH : public FiducialDetectorPluginForQuads{
      struct Data;
      Data *data;

      /// only the FiducialDetector can instantiate this class
      FiducialDetectorPluginBCH();
      public:

      /// This class cannot be used
      friend class icl::markers::FiducialDetector;

      /// Destructor
      ~FiducialDetectorPluginBCH();

      /// loads markers ID's
      /** @param add
          @param which this any instance can be ...
          * of type int (then, only the corresponding marker ID is loaded)
          * of type utils::Range32s "[a,b]", (then all markers within the range are loaded)
          * of something like {a,b,c,d,...} then all marker IDs in the list are loaded

          Please note that other types might be interpreted in the wrong way.
          Mandatory parameter is "size". Please refer to the
          documentation of icl::markers::FiducialDetector::loadMarkers for more details
         @param params
      */
      virtual void addOrRemoveMarkers(bool add, const utils::Any &which, const utils::ParamList &params);

      /// extracts and stores some properties locally to speed up classifyPatch
      void prepareForPatchClassification();

      /// Identifies the given image patch using bch decoding
      virtual FiducialImpl *classifyPatch(const core::Img8u &image, int *rot, bool returnRejectedQuads, cv::ImageRegion r);

      /// describes the marker rectification parameters
      virtual void getQuadRectificationParameters(utils::Size &markerSizeWithBorder,
                                                  utils::Size &markerSizeWithoutBorder);

      /// creates bch marker image
      virtual core::Img8u createMarker(const utils::Any &whichOne,const utils::Size &size, const utils::ParamList &params);

    };
  } // namespace icl::markers