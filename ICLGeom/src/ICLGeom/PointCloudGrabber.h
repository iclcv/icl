// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Patrick Nobou

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
      virtual const core::Img32f *getDepthDisplay() const { return 0; }

      /// returns the last grabbed point cloud's underlying depth image (if available)
      virtual const core::Img8u *getColorDisplay() const { return 0; }

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
