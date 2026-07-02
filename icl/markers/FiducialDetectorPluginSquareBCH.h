// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/markers/FiducialDetectorPluginForQuads.h>

namespace icl::markers {
  /** \cond */
  class FiducialDetector;
  /** \endcond */

  /// FiducialDetectorPlugin for square (n x n) BCH markers \ingroup PLUGINS
  /** The n x n generalization of FiducialDetectorPluginBCH: same quad-based
      pipeline (inherited from FiducialDetectorPluginForQuads), but the marker
      interior is a SquareBCHCode of a configurable grid size (3x3 / 4x4 / 5x5),
      not the fixed 6x6 BCH code. Used for the smaller, lower-cardinality markers
      that pack into dense coded calibration targets. Instantiated by
      FiducialDetector via the marker types "bch3x3", "bch4x4", "bch5x5". */
  class ICLMarkers_API FiducialDetectorPluginSquareBCH : public FiducialDetectorPluginForQuads{
    struct Data;
    Data *data;

    /// only the FiducialDetector can instantiate this class
    /** \a gridSize is n (3..6), \a correctable the BCH error-correction t. */
    FiducialDetectorPluginSquareBCH(int gridSize, int correctable);
    public:

    friend class icl::markers::FiducialDetector;

    ~FiducialDetectorPluginSquareBCH();

    /// loads marker IDs (int / range "[a,b]" / list "{a,b,c}"); "size" is mandatory
    virtual void addOrRemoveMarkers(bool add, const std::string &which, const utils::ParamMap &params);

    /// caches properties locally to speed up classifyPatch
    void prepareForPatchClassification();

    /// identifies the given (rectified) image patch via SquareBCHCode decoding
    virtual FiducialImpl *classifyPatch(const core::Img8u &image, int *rot, bool returnRejectedQuads, cv::ImageRegion r);

    /// marker rectification sizes (n code cells + border cells, times match factor)
    virtual void getQuadRectificationParameters(utils::Size &markerSizeWithBorder,
                                                utils::Size &markerSizeWithoutBorder);

    /// creates a square-BCH marker image
    virtual core::Img8u createMarker(const std::string &whichOne,const utils::Size &size, const utils::ParamMap &params);

  };
  } // namespace icl::markers
