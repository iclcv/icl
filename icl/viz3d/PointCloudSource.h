// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <memory>
#include <string>

#ifndef ICLViz3d_API
#define ICLViz3d_API
#endif

namespace icl::utils { class ProgArg; }
namespace icl::core { class Image; }
namespace icl::cv3d { class Camera; }
namespace icl::io { class ImageSource; }

namespace icl::viz3d {

  class PointCloud;

  /// Turns an image-based depth/RGBD ImageSource into viz3d point clouds.
  /** The viz3d/ImageSource counterpart of the legacy GenericPointCloudGrabber:
      wraps an io::ImageSource yielding a float depth image (1 channel = depth
      in mm) or RGBD image (4 channels = R,G,B,depth), resolves the depth camera
      (from the frame's metadata, or set explicitly), and reconstructs a
      PointCloud via PointCloud::unprojectDepth.

      Shared by icl-point-cloud-viewer and icl-point-cloud-pipe so the
      grab → extract → unproject logic lives in one place. */
  class ICLViz3d_API PointCloudSource {
  public:
    PointCloudSource();
    ~PointCloudSource();

    /// Initialise the underlying ImageSource (e.g. from the -i program arg).
    void init(const utils::ProgArg &pa);
    void init(const std::string &device, const std::string &spec);

    /// Pin the depth camera (overrides any camera carried in frame metadata).
    void setCamera(const cv3d::Camera &cam);
    bool hasCamera() const;
    const cv3d::Camera &getCamera() const;

    /// Depth interpretation: true = Z-depth (distance to image plane, default),
    /// false = Euclidean distance to the camera centre.
    void setDistToCamPlane(bool enabled);

    /// Grab the next frame and reconstruct it into \a dst.
    /** Returns false if the frame carried no usable float depth, or no camera
        was available yet (neither set nor in metadata). */
    bool grab(PointCloud &dst);

    /// The most recently grabbed raw frame (for relaying / output).
    const core::Image &getLastFrame() const;

    /// The underlying source (e.g. to forward properties or grab the raw frame).
    io::ImageSource &getImageSource();

  private:
    struct Data;
    std::unique_ptr<Data> m_data;
  };

} // namespace icl::viz3d
