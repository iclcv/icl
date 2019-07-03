/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/PointCloudGrabber.h                **
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
#include <ICLUtils/Configurable.h>
#include <ICLGeom/PointCloudObjectBase.h>
#include <ICLGeom/Camera.h>
#include <ICLMath/FixedMatrix.h>

#include <map>

namespace icl{
  namespace geom{

    /// Generic interface for PointCloud sources
    struct PointCloudGrabber : public utils::Configurable{
      /// fills the given point cloud with grabbed information
      virtual void grab(PointCloudObjectBase &dst) = 0;

      /// virtual, but empty destructor
      virtual ~PointCloudGrabber(){}

      /// returns the last grabbed point cloud's underlying depth image (if available)
      virtual const core::Img32f *getDepthImage() const { return 0; }

      /// returns the last grabbed point cloud's underlying depth image (if available)
      virtual const core::Img8u *getColorImage() const { return 0; }

      /// returns current depth camera (CAN be implemented by implementation
      virtual Camera getDepthCamera() const{
        throw utils::ICLException("PointCloudGrabber::getDepthCamera() is not implemented by current backend");
      }

      virtual Camera getColorCamera() const{
        throw utils::ICLException("PointCloudGrabber::getColorCamera() is not implemented by current backend");
      }

      virtual void setCameraWorldFrame(const math::FixedMatrix<float,4,4> &T){
        throw utils::ICLException("PointCloudGrabber::setCameraWorldFrame() is not implemented by current backend");
      }


      /// re-initializes the current device
      /** The backend can choose to throw an exception */
      virtual void reinit(const std::string &description){
        throw utils::ICLException("reinit is not implemented for this PointCloudGrabber backend type");
      }
    };
  }
}

