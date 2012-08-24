/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLGeom/DepthCameraPointCloudGrabber.h         **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Patrick Nobou                     **
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

#pragma once

#include <ICLGeom/PointCloudGrabber.h>

#include <ICLGeom/Camera.h>

namespace icl{
  /// PointCloudGrabber implementation for 2D depth-image based creation of point clouds
  /** This Grabber implementation can be used for all point-cloud sources,
      where the point cloud must still be created from a given depth images.
      Internally an instance of PointCloudCreator is used. */
  class DepthCameraPointCloudGrabber : public PointCloudGrabber{
    struct Data;  //!< pimpl type
    Data *m_data; //!< pimpl pointer
      
    public:
    /// constructor with a set of given parameters
    /** @param depthCam depth camera parameters (if nothing is passed, a simple
                        VGA camera is used automatically)
        @param colorCam color camera parameters. If get_null_color_cam() is passed,
                        then no color camera information will be available, no
                        color images will be grabbed and only xyz point cloud data
                        is created (all other fiels are just left untouched)
        
        depthDeviceType and ID: device type (e.g. kinectd for Kinect source of file 
        filepattern for using a list of source files that contained
        depth images) Analogously for colorDeviceType and ID, however the latter two
        are ignored if not colorCam was passed.
    */
    DepthCameraPointCloudGrabber(const Camera &depthCam=get_default_depth_cam(),
                                 const Camera &colorCam=get_null_color_cam(),
                                 const std::string &depthDeviceType="kinectd",
                                 const std::string &depthDeviceID="0",
                                 const std::string &colorDeviceType="kinectc",
                                 const std::string &colorDeviceID="0");
    
    /// Destructor
    ~DepthCameraPointCloudGrabber();
  
    /// virtual grab implementation
    virtual void grab(PointCloudObjectBase &dst);
    
    /// returns the last grabbed depth image
    const Img32f &getLastDepthImage() const;
    
    /// returns the last grabbed color image
    /** Throws an exception if no color camera or not valid color camera device
        type and ID were passed. */
    const Img8u &getLastColorImage() const throw (ICLException);
    
    /// creates the defautl VGA depth camera
    static const Camera &get_default_depth_cam();

    /// creates the defautl null color camera
    /** The camera is never really used, only it's address is used to decide
        whether it is the static null-color-camera or not */
    static const Camera &get_null_color_cam();
  };
}

