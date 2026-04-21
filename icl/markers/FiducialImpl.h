// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Size32f.h>
#include <icl/markers/Fiducial.h>

#include <bitset>

namespace icl::markers {
  /// Hidden implemetation for fiduical classes
  /** The FiducialImpl class works as data storage for the public
      Fiducial class. It also holds a back-reference to it's parent
      FiducialDetectorPlugin that is used to compute features
      on demand if not alread available

      \section DEV Developers
      The FiduicalImpl should not be visible for the FiduicalDetector user
      who accesses Fiducial information via the public Fiduical class.
      The Fiduical class implementation contains smart methods for
      querying Fiducial-Features. If a feature has been computed once,
      further calls will just return the automatically buffered result.

      \section PP Precomputed vs. Deferred Feature Extraction
      The feature Extraction is implemented in the FiducialDetectorPlugin
      class that defines
      * how Fiducials are detected
      * what features are supported
      * how certain features are computed.

      The FiducialDetectorPlugin <b>can</b> initialize the FiduicalImpl instances
      with precomputed values. Most of the time, at least some of the
      Fiducial feature values are already estimated within the detection step.
      If e.g. a region based fiducial detector needs to estimate a regions 2D-center
      for the detection itself, it can initialize the FiducialInstances with
      these values. Of course, it has to set up the 'computed' mask in the
      FiduicalImpl instances according to the set of precomputed features.\n
      Other features, that are supported, but perhaps more difficult to compute
      can in contrast be supported in a deferred manner. The FiducialDetectorPlugin
      will the leave the 'computed' mask empty for this feature and provide
      an appropriate getter-method implementation. This is in particular useful
      to speed up the detection step itself since usually most complex features
      dont have to be computed for each detected Fiducial.
  */
  struct ICLMarkers_API FiducialImpl{
    /// parent Fiducial Detector instance
    FiducialDetectorPlugin *parent;

    /// list of generally available features
    Fiducial::FeatureSet supported;

    /// list of already computed features
    Fiducial::FeatureSet computed;

    /// Fiduical ID
    /*  The Fiduical ID can be used externally to distinguish between differnent
        Fiducial instances */
    int id;

    /// Internally used index
    /** The index can be estimate a FiducialImpl instances index from
        the getFeature-methods in the FiducialDetectorPlugin class */
    int index;

    /// real marker size in millimeters
    /** This information is neccessary for creation of feature points and
        for most 3D pose detection stuff */
    utils::Size32f realSizeMM;

    /// most plugins can provide an image region
    cv::ImageRegion imageRegion;

    /// set of 2D features
    /* this pointer and its members <b>can</b> be instantiated by
        parent FiduicalDetectorImpl instance */
    struct Info2D{
      utils::Point32f infoCenter;
      float infoRotation;
      std::vector<utils::Point32f> infoCorners;
      std::vector<Fiducial::KeyPoint> infoKeyPoints;
    } *info2D;

    /// set of 3D features
    /* this pointer and its members <b>can</b> be instantiated by
        parent FiduicalDetectorImpl instance */
    struct Info3D{
      geom::Vec infoCenter;
      geom::Vec infoRotation;
      geom::Mat infoPose;
    } *info3D;

    /// base constructor initializing index with -1 and all other stuff with 0
    inline FiducialImpl():supported(0),computed(0),id(-1),index(-1),info2D(0),info3D(0){}

    /// 2nd constructor with given parameters
    inline FiducialImpl(FiducialDetectorPlugin *parent,
                        Fiducial::FeatureSet supported,
                        Fiducial::FeatureSet computed,
                        int id, int index,
                        const utils::Size32f &realSizeMM):
      parent(parent),supported(supported),
      computed(computed),id(id),index(index),
      realSizeMM(realSizeMM),info2D(0),info3D(0){}

    /// explicitly implemented deep copy constructor
    FiducialImpl(const FiducialImpl &o);

    /// explicitly implemented assignment operator (deep copy)
    FiducialImpl &operator=(const FiducialImpl &o);

    /// Destructor (also deletes info2D and info3D if not null)
    inline ~FiducialImpl(){
      if(info2D) delete info2D;
      if(info3D) delete info3D;
    }

    /// ensures that the info2D pointer is initialized
    inline Info2D *ensure2D(){
      if(!info2D) info2D = new Info2D;
      return info2D;
    }

    /// ensures that the info3D pointer is initialized
    inline Info3D *ensure3D(){
      if(!info3D) info3D = new Info3D;
      return info3D;
    }

  };


  } // namespace icl::markers