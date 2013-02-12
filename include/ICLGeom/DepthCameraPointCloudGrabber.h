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
  namespace geom{
    /// PointCloudGrabber implementation for 2D core::depth-image based creation of point clouds
    /** This Grabber implementation can be used for all point-cloud sources,
        where the point cloud must still be created from a given core::depth images.
        Internally an instance of PointCloudCreator is used. */
    class DepthCameraPointCloudGrabber : public PointCloudGrabber{
      struct Data;  //!< pimpl type
      Data *m_data; //!< pimpl pointer
        
      public:
      /// constructor with a set of given parameters
      /** @param depthCam core::depth camera parameters (if nothing is passed, a simple
                          VGA camera is used automatically)
          @param colorCam color camera parameters. If get_null_color_cam() is passed,
                          then no color camera information will be available, no
                          color images will be grabbed and only xyz point cloud data
                          is created (all other fiels are just left untouched)
          
          @param depthDeviceType, depthDeviceID: device type (e.g. kinectd for Kinect source of file 
          filepattern for using a list of source files that contained
          core::depth images) Analogously for <b>colorDeviceType</b> and <b>colorDeviceID</b>, however the latter two
          are ignored if not colorCam was passed.
          @param colorDeviceType, colorDeviceID (See <b>depthDeviceType</b> and <b>depthDeviceID</b>)
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
      
      /// returns the last grabbed core::depth image
      const core::Img32f &getLastDepthImage() const;
      
      /// returns the last grabbed color image
      /** Throws an exception if no color camera or not valid color camera device
          type and ID were passed. */
      const core::Img8u &getLastColorImage() const throw (utils::ICLException);
      
      /// creates the defautl VGA core::depth camera
      static const Camera &get_default_depth_cam();
  
      /// creates the defautl null color camera
      /** The camera is never really used, only it's address is used to decide
          whether it is the static null-color-camera or not */
      static const Camera &get_null_color_cam();

      /// maps another given image just like the rgbImage would be mapped
      /** @param src image assumed to be captured from the perspective of the color camera
          @param dst destimation image (automatically adapted)
          @param depthImageMM optionally given depth image (if NULL, then the last
          depthImage passed to the "create"-is used, which should usually be the right one) */
      void mapImage(const core::ImgBase *src, core::ImgBase **dst, const core::Img32f *depthImageMM=0);

      /// defines whether opencl is to be used
      /** Please note that OpenCL is only used if
          * The graphics card supports OpenCL
          * ICL is build with OpenCL support 
          * for the most common point cloud types (i.e. color type is rgba32f)
          * color- and depth-camera size are equal */
      void setUseCL(bool enable);
    };
  } // namespace geom
}

