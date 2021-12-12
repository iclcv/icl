/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/GenericPointCloudGrabber.h         **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter                                    **
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

#include <ICLGeom/PointCloudGrabber.h>
#include <ICLUtils/ProgArg.h>
#include <ICLUtils/Configurable.h>

namespace icl{
  namespace geom{

    /// Generic interface for PointCloud sources
    class ICLGeom_API GenericPointCloudGrabber : public PointCloudGrabber {
      struct Data;
      Data *m_data;

      public:

      /// Empty constructor (creates a null instance)
      GenericPointCloudGrabber();

      /// Constructor with initialization
      /** Possible plugins:
          * <b>dcam</b> device description is then: "depth-cam-type,depth-cam-id,depth-cam-file"
            * optionally ",color-cam-type,color-cam-id,color-cam-file"
            * an additional comma-seperated token "raw" can be passed to make the grabber
              compatible to Kinect11BitRaw depth input images
          * <b>file</b> filename pattern (not yet implemented)
          * <b>rsb</b> [rsb-transport-list:]rsb-scope-list[,depth-cam-filename[,color-cam-filename]]
      */
      GenericPointCloudGrabber(const std::string &sourceType, const std::string &srcDescription);

      /// direct initialization from program argument
      /** Prog-arg is assumed to have 2 sub-args */
      GenericPointCloudGrabber(const utils::ProgArg &pa);

      /// destructor
      ~GenericPointCloudGrabber();

      /// deferred intialization
      void init(const std::string &sourceType, const std::string &srcDescription);

      /// re-initializes the current device
      /** The backend can choose to throw an exception. The syntax
          for reinitialization is defined by each backend individually */
      void reinit(const std::string &description);

      /// forwards call to current backend
      Camera getDepthCamera() const;

      /// forwards call to current backend
      Camera getColorCamera() const;

      /// forwards call to current backend
      void setCameraWorldFrame(const math::FixedMatrix<float,4,4> &T);


      /// deferred initialization from ProgArg (most common perhaps)
      /** Prog-arg is assumed to have 2 sub-args */
      void init(const utils::ProgArg &pa);

      /// not initialized yet?
      bool isNull() const;

      /// fills the given point cloud with grabbed information
      virtual void grab(PointCloudObjectBase &dst);

      /// returns the last grabbed point cloud's underlying depth image (if available)
      virtual const core::Img32f *getDepthImage() const;

      /// returns the last grabbed point cloud's underlying depth image (if available)
      virtual const core::Img8u *getColorImage() const;

    };
  } // namespace geom
}
