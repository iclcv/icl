// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/viz3d/pointcloud/PointCloudSource.h>
#include <icl/viz3d/pointcloud/PointCloud.h>
#include <icl/io/source/ImageSource.h>
#include <icl/cv3d/Camera.h>
#include <icl/core/Image.h>
#include <icl/core/Img.h>
#include <sstream>
#include <algorithm>

namespace icl::viz3d {

  struct PointCloudSource::Data {
    io::ImageSource source;
    cv3d::Camera cam;
    bool haveCam = false;
    bool fixedCam = false;          // camera was set explicitly (ignore metadata)
    bool distToCamPlane = true;
    core::Image lastFrame;
    core::Img32f depthBuf;          // depth view extracted from the frame
    core::Img8u  colorBuf;          // colour extracted from a packed RGBD frame
  };

  PointCloudSource::PointCloudSource() : m_data(std::make_unique<Data>()) {}
  PointCloudSource::~PointCloudSource() = default;

  void PointCloudSource::init(const utils::ProgArg &pa) { m_data->source.init(pa); }
  void PointCloudSource::init(const std::string &device, const std::string &spec) {
    m_data->source.init(device, spec);
  }

  void PointCloudSource::setCamera(const cv3d::Camera &cam) {
    m_data->cam = cam; m_data->haveCam = true; m_data->fixedCam = true;
  }
  bool PointCloudSource::hasCamera() const { return m_data->haveCam; }
  const cv3d::Camera &PointCloudSource::getCamera() const { return m_data->cam; }
  void PointCloudSource::setDistToCamPlane(bool enabled) { m_data->distToCamPlane = enabled; }

  const core::Image &PointCloudSource::getLastFrame() const { return m_data->lastFrame; }
  io::ImageSource &PointCloudSource::getImageSource() { return m_data->source; }

  bool PointCloudSource::grab(PointCloud &dst) {
    core::Image img = m_data->source.grab();
    m_data->lastFrame = img;

    // Resolve the depth camera (fixed via setCamera, else per-frame metadata).
    if (!m_data->fixedCam && img.ptr()->hasMetaData()) {
      std::istringstream is(img.ptr()->getMetaData());
      is >> m_data->cam;
      m_data->haveCam = true;
    }
    if (!m_data->haveCam || img.getDepth() != core::depth32f) return false;

    const core::Img32f &src = img.as<icl32f>();
    const core::Img8u *color = nullptr;
    if (img.getChannels() == 1) {
      m_data->depthBuf = src;                                   // depth-only
    } else if (img.getChannels() >= 4) {                        // packed R,G,B,depth
      const int dim = src.getDim();
      m_data->depthBuf.setSize(src.getSize());
      m_data->depthBuf.setChannels(1);
      std::copy(src.getData(3), src.getData(3) + dim, m_data->depthBuf.getData(0));
      m_data->colorBuf = core::Img8u(src.getSize(), core::formatRGB);
      for (int c = 0; c < 3; ++c) {
        const float *s = src.getData(c);
        icl8u *d = m_data->colorBuf.getData(c);
        for (int i = 0; i < dim; ++i) d[i] = (icl8u)std::clamp(s[i], 0.f, 255.f);
      }
      color = &m_data->colorBuf;
    } else {
      return false;
    }

    dst.unprojectDepth(m_data->depthBuf, m_data->cam, m_data->distToCamPlane, color);
    return true;
  }

} // namespace icl::viz3d
