// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLMarkers/Fiducial.h>
#include <ICLGeom/Camera.h>

namespace icl::markers {
  /// Internal Implementation class for the MutiCamFiducial
  /** @section _SB_ Smart Buffering
      The 3D pose estimation results are buffered internally so that
      mutiple calls to MutiCamFiducial::getPose3D do not entail
      doubled pose estimation.
  */
  struct ICLMarkers_API MultiCamFiducialImpl{
    int id;                              //!< associated fiducial ID
    int numFound;                        //!< number of view, this Fiducial was found in
    std::vector<Fiducial> fids;          //!< all 2D fiducials
    std::vector<geom::Camera*> cams;           //!< all cameras
    math::FixedColVector<float,3> center;      //!< smart buffer for the center
    math::FixedColVector<float,3> orientation; //!< smart buffer for the orientation
    geom::Mat pose;                            //!< smart buffer for the pose
    bool haveCenter;      //!< has the center already been estimated
    bool haveOrientation; //!< has the orientation already been estimated
    bool havePose;        //!< has the pose already been estimated

    /// null/empty constructor
    MultiCamFiducialImpl();

    /// default constructor with given ID
    MultiCamFiducialImpl(int id,
                         const std::vector<Fiducial> &fids,
                         const std::vector<geom::Camera*> cams);


    /// (re-) initialization
    void init(int id);

    /// estimate and return the 3D center
    const math::FixedColVector<float,3> &estimateCenter3D();

    /// estimate and return the 3D pose
    const geom::Mat &estimatePose3D();

    /// estimate and return the 3D orientation
    const math::FixedColVector<float,3> &estimateOrientation3D();
  };

  } // namespace icl::markers