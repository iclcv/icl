/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/PointCloudCreator.h                **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Patrick Nobou                     **
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
#include <ICLGeom/PointCloudObjectBase.h>
#include <ICLGeom/Camera.h>
#include <ICLGeom/RGBDMapping.h>

namespace icl{
  namespace geom{
    
    /// Utility class that allows to create 3D (optionally colored) point clouds from given 2D core::depth images
    /** The PointCloudCreator utility class can be used to two differenct modes.
        It's basic functionality is to transform a core::depth-image into a 3D point cloud. 
        For this, a Camera instances is needed, which can be obtained from ICL's
        camera calibration tool icl-cam-calib-2 for real core::depth cameras such as Kinect.
        
        If also a second camera file is provided for the color camera, then
        the PointCloudCreator class can also be used for mapping optionally given
        RGB-byte images to the contained point's colors.
  
        \section _SPEED_ Benchmarks
        For VGA point clouds, creation with RGBD mapping takes about 8ms on a 2.5 GHz Core2Duo machine.
        If RGBD-mapping is not performed, the simple point cloud creation is performed in about 6 ms.
        For futher speed optimizations, we plan to add openmp optimization in the future.
    */
    class ICLGeom_API PointCloudCreator{
      struct Data;  // !< pimpl type
      Data *m_data; // !< pimpl pointer
      public:
      
      /// Representation of the given core::depth images
      /** Depth image values can either be understood as distance values
          to the the camera center or as distances from the camera's viewing
          plane */
      enum DepthImageMode{
        DistanceToCamCenter, //!< distances are given w.r.t. the camera center
        DistanceToCamPlane,  //!< distances are given w.r.t. the camera plance
        KinectRAW11Bit       //!< default kinect raw values in range [0,2047]
      };
  
      /// creates a null instance
      PointCloudCreator();
      
      /// creates a new instance with given core::depth camera (no rgbd mapping is available then)
      PointCloudCreator(const Camera &depthCam, DepthImageMode mode=DistanceToCamPlane);
  
      /// creates a new instance with given core::depth camera and color camera for rgbd mapping
      PointCloudCreator(const Camera &depthCam, const Camera &colorCam, DepthImageMode mode=DistanceToCamPlane);
  
      /// deep copy constructor
      PointCloudCreator(const PointCloudCreator &other);
  
      /// deep copy assginment operator
      PointCloudCreator &operator=(const PointCloudCreator &other);

      
      /// Destructor
      ~PointCloudCreator();
      
      /// initializes with given core::depth camera (no rgbd mapping is available then)
      void init(const Camera &depthCam, DepthImageMode mode=DistanceToCamPlane);
      
      /// initializes with given core::depth camera and color camera for rgbd mapping
      void init(const Camera &depthCam, const Camera &colorCam, DepthImageMode mode=DistanceToCamPlane);
      
      /// creates a point cloud
      /** If the PointCloudCreator instance is set up to provide color information and the destionation point cloud
          does not support color information, the detination point cloud is tried to be adapted using
          PointCloudObjectBase::canAddFeature and PointCloudObjectBase::addFeature */
      void create(const core::Img32f &depthImage, PointCloudObjectBase &destination, const core::Img8u *rgbImage = 0, float depthScaling=1);
  
      /// returns the current core::depth camera
      const Camera &getDepthCamera() const;
      
      /// returns the current camera camera (if this was not given, an exception is thrown)
      const Camera &getColorCamera() const throw (utils::ICLException);

      /// sets new cameras (reinitializes data structures internally)
      /** This functions is relatively complex, so it should be performed only
          when it is needed */
      void setCameras(const Camera &depthCam, const Camera &colorCam);

      /// sets the internally used depthCamera
      /** This functions is relatively complex, so it should be performed only
          when it is needed.
          After using this function, the creator will have no color camera any more*/
      void setDepthCamera(const Camera &depthCam);
      
      /// returns whether a color camera was given (and therefore whether RGBD-mapping is supported)
      bool hasColorCamera() const;

      /// maps another given image just like the rgbImage would be mapped
      /** @param src image assumed to be captured from the perspective of the color camera
          @param dst destimation image (automatically adapted)
          @param depthImageMM optionally given depth image (if NULL, then the last
          depthImage passed to the "create"-is used, which should usually be the right one) */
      void mapImage(const core::ImgBase *src, core::ImgBase **dst, const core::Img32f *depthImageMM=0);
      
      /// Enables/disables openCL accelaration
      /** In case of having no opencl support, this function does nothing */
      void setUseCL(bool use);  
      
      /// returns the internal mapping
      /** Only if both- depth and camera camera parameters are available.
          Please note, that the returned shallowly copies the internal 
          depth camera viewray array */
      RGBDMapping getMapping() const throw (utils::ICLException);

      /// sets up internal heuristical fixes applied to the used depth camera parameters
      /** @param focalLengthMultiplier is used as multiplicative adaption to the orginal depth camera's 
                 focal length. 1 is the neutral value for this 
          @param positionOffsetAlongNorm is used as additive displacement for the used depth camera along
                 the camera's view normal
          */
      void setFixes(float focalLengthMultiplier=1, float positionOffsetAlongNorm=0);
    };
  
  } // namespace geom
}

