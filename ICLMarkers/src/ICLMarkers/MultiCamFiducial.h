// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLMarkers/FiducialDetector.h>
#include <ICLGeom/GeomDefs.h>

namespace icl{
  namespace markers{

    /** \cond */
    struct MultiCamFiducialImpl;
    /** \endcond */


    /// Fiducial Type for the MuliCamFiducialDetector
    /** MultiCamFiducials provide much more robust 3D information
        if they a constructed from several camera views

        The MultiCamFiducialDetector provides a generic interface
        to detect fiducials in a set of camera views

        \section __MEM__ Memory management
        The MultiCamFiducial is implemented as a wrapper around
        a single implementation pointer whose memory is managed
        by the parent MultiCamFiducialDetector instance. Therefore
        MultiCamFiducial instances can copied like objects

        \section __3D_EST__ 3D Pose estimation
        For the pose estimation, two different algorithms are combined.
        In case of Fiducials, that were just detected in a single
        camera view, the CoplanarPointPoseEstimator is used. If more
        than one view is available, multicamera geometry is used to
        provide a much moch accurate estimation of the marker pose.
    */
    class ICLMarkers_API MultiCamFiducial{
      MultiCamFiducialImpl *impl; //!< internal implementation

        public:
      /// Constructor
      MultiCamFiducial(MultiCamFiducialImpl *impl=0):impl(impl){}

      /// returns whether this is null-instance
      bool isNull() const { return !impl; }

      /// returns the number of cameras this fiducial was actually detected in
      /** The number of cameras can be a good indicator for the accuracy of the
          3D position returned by getCenter3D(), getOrientation3D() and getPose3D() */
      int getCamsFound() const;

      /// returns the underlying 2D fiducial detection result
      /** valid indices are [0,getCamsFound()-1]*/
      Fiducial operator[](int idx);

      /// returns the underlying 2D fiducial detection retult (const version)
      const Fiducial operator[](int idx) const;

      /// returns the idx-th associated camera
      /** Note, the camera can also be accessed from the
          idx-th Fiducial*/
      geom::Camera &getCamera(int idx);

      /// returns the idx-th associated camera (const version)
      const geom::Camera &getCamera(int idx) const;

      /// returns the associated marker ID
      int getID() const;

      /// returns the 3D center of the marker
      /** @see \ref __3D_EST__ */
      const math::FixedColVector<float,3> &getCenter3D() const;

      /// returns the 3D orientation of the marker
      /** @see \ref __3D_EST__ */
      const math::FixedColVector<float,3> &getOrientation3D() const;

      /// returns the 6D-pose (in shape of a homogeneous transform) of the marker
      /** @see \ref __3D_EST__ */
      const geom::Mat &getPose3D() const;
    };

  } // namespace markers
}
