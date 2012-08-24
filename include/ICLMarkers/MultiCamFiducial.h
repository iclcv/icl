/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLMarkers/MultiCamFiducial.h                  **
** Module : ICLCV                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICL_MULTICAM_FIDUCIAL_H
#define ICL_MULTICAM_FIDUCIAL_H

#include <ICLMarkers/FiducialDetector.h>
#include <ICLGeom/GeomDefs.h>

namespace icl{
  
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
  class MultiCamFiducial{
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
    Camera &getCamera(int idx);
    
    /// returns the idx-th associated camera (const version)
    const Camera &getCamera(int idx) const;
    
    /// returns the associated marker ID
    int getID() const;
    
    /// returns the 3D center of the marker
    /** @see \ref __3D_EST__ */
    const FixedColVector<float,3> &getCenter3D() const;
    
    /// returns the 3D orientation of the marker
    /** @see \ref __3D_EST__ */
    const FixedColVector<float,3> &getOrientation3D() const;
    
    /// returns the 6D-pose (in shape of a homogeneous transform) of the marker
    /** @see \ref __3D_EST__ */
    const Mat &getPose3D() const;
  };

}

#endif
