// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Andre Ueckermann, Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/Img.h>
#include <ICLFilter/UnaryOp.h>
#include <ICLCore/Image.h>
#include <vector>

namespace icl::filter {
/// Temporal smoothing filter with motion detection.
///
/// Maintains a ring buffer of recent frames per channel. For each pixel,
/// computes the temporal average across the buffer, unless the range
/// (max - min) exceeds a configurable threshold — in that case the
/// current frame value is passed through (motion detected).
///
/// Supports depth8u and depth32f only. Full-image ROI required.
/// Designed for depth camera data (e.g. Kinect) where a configurable
/// null value indicates missing data.
class ICLFilter_API MotionSensitiveTemporalSmoothing : public UnaryOp {
public:
  MotionSensitiveTemporalSmoothing(const MotionSensitiveTemporalSmoothing&) = delete;
  MotionSensitiveTemporalSmoothing& operator=(const MotionSensitiveTemporalSmoothing&) = delete;

  /// @param nullValue pixel value indicating no data (-1 = no null values)
  /// @param maxFilterSize maximum temporal window size (ring buffer capacity)
  MotionSensitiveTemporalSmoothing(int nullValue, int maxFilterSize);
  ~MotionSensitiveTemporalSmoothing();

  void apply(const core::Image &src, core::Image &dst) override;
  using UnaryOp::apply;

  /// Enable/disable OpenCL acceleration (reserved for future use)
  void setUseCL(bool use);
  /// Returns whether OpenCL is currently enabled
  bool isCLActive() const;

  void setFilterSize(int filterSize);
  void setDifference(int difference);

  int getFilterSize() const { return m_filterSize; }
  int getDifference() const { return m_difference; }
  int getNullValue() const { return m_nullValue; }
  int getMaxFilterSize() const { return m_maxFilterSize; }

  /// Returns the motion image (255=motion, 0=static) for the first channel
  core::Img32f getMotionImage() const;
  /// Legacy alias for getMotionImage()
  core::Img32f getMotionDisplay() { return getMotionImage(); }

private:
  struct ChannelState {
    std::vector<core::Img32f> historyF;
    std::vector<core::Img8u> historyC;
    core::Img32f motionImage;
    int imgCount = 0;
  };

  void reinit(int channels, core::depth d, utils::Size size);

  int m_nullValue;
  int m_maxFilterSize;
  int m_filterSize;
  int m_difference;
  bool m_useCL;

  int m_numChannels = 0;
  utils::Size m_size;
  core::depth m_depth = core::depth8u;

  std::vector<ChannelState> m_channels;
};

} // namespace icl::filter