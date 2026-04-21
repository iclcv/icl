// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/markers/FiducialDetectorPlugin.h>
#include <icl/markers/QuadDetector.h>

namespace icl::markers {
  /** \cond */
  class FiducialDetector;
  /** \endcond */


  /// FiducialDetectorPlugin for quad-markers like ARToolkit and BCH-Code markers \ingroup PLUGINS
  class ICLMarkers_API FiducialDetectorPluginForQuads : public FiducialDetectorPlugin{
    /// Internal Data
    struct Data;

    /// Internal data pointer
    Data *data;

    protected:

    /// only the FiducialDetector can instantiate this class
    FiducialDetectorPluginForQuads();
    public:

    /// This class cannot be used
    friend class icl::markers::FiducialDetector;

    /// Destructor
    ~FiducialDetectorPluginForQuads();

    // use the edges
    virtual void getKeyPoints2D(std::vector<Fiducial::KeyPoint> &dst, FiducialImpl &impl);
    virtual void getFeatures(Fiducial::FeatureSet &dst);
    virtual void detect(std::vector<FiducialImpl*> &dst, const core::Img8u &image);

    /// this plugin uses the binarisation from the internally used quad-detector
    virtual SourceImageType getPreProcessing() const {  return Gray;  }

    /// loads markers ID's (also implemented in the subclasses)
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
    virtual void addOrRemoveMarkers(bool add, const utils::Any &which, const utils::ParamList &params) = 0;

    /// this plugin provides some extra intermediate images
    std::string getIntermediateImageNames() const;

    /// returns the intermediate image, that is associated with the given name
    /** @see getIntermediateImageNames for more details */
    const core::ImgBase *getIntermediateImage(const std::string &name) const;

    /// this method is called before the patch classification loop is started
    /** this function can be used to avoid property extraction at runtime. Usually,
        a certain implementation can read out and store all property values that are used
        in classify patch once in a whole image processing cylce */
    virtual void prepareForPatchClassification(){}

    /// this method must be called in the subclasses
    virtual FiducialImpl *classifyPatch(const core::Img8u &image, int *rot, bool returnRejectedQuads, cv::ImageRegion r) = 0;

    /// this method is also implemented in the subclasses
    /** The method describes the parameters for the marker rectification */
    virtual void getQuadRectificationParameters(utils::Size &markerSizeWithBorder,
                                                utils::Size &markerSizeWithoutBorder) = 0;

    /// returns the internal quad-detector
    QuadDetector& getQuadDetector();

  };
  } // namespace icl::markers