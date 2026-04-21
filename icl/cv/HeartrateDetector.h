// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Matthias Esau, Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Uncopyable.h>
#include <icl/core/Img.h>
#include <icl/qt/PlotWidget.h>
namespace icl::cv {
  /// Heartrate Detector
  /** Implementation of a simple heartrate detector using video material of a face.

      The algorithm takes images cropped to the same part of the face as an input and
      estimates the heartrate using a Fast Fourier Transformation over the average green
      value of the past few frames.

      Parameters for the algorithm are:
    - <b>framrate</b>
      denotes the framerate of the source material so the algorithm
      can correctly estimate the heartrate.
    - <b>historyDepth</b>
      denotes how many frames are sampled before the algorithm starts
      estimating the heartrate. Using a larger historyDepth allows for
      a more fine grained estimation of the heartrate, but introduces a
      bigger delay before a first estimate can be given.

      usage example:
      \code
      icl::cv::HeartrateDetector detector(30,512);
      detector.addImage(image);
      float heartrate = detector.getHeartrate();
      \endcode
  **/
  class ICLCV_API HeartrateDetector {

    struct Data;  //!< internal data structure
    Data *m_data; //!< internal data pointer

    public:
    HeartrateDetector(const HeartrateDetector&) = delete;
    HeartrateDetector& operator=(const HeartrateDetector&) = delete;

    /// Default constructor with given arguments.
    HeartrateDetector(int framerate = 30, int historyDepth = 512);

    /// Add an image to the detector to use in the estimation.
    void addImage(const core::Img8u &image);
    /// returns the current estimate for the heartrate. Will return 0
    /// if no enough enough samples have been provided.
    float getHeartrate() const;
    /// returns a SeriesBuffer for visualization of the frequencies.
    qt::PlotWidget::SeriesBuffer getFrequencies() const;
    /// returns a SeriesBuffer for visualization of the averaged
    /// frequencies over the last second.
    qt::PlotWidget::SeriesBuffer getAveragedFrequencies() const;
    /// returns a buffer that marks the current window of frequencies
    /// that is considered for the estimation of the heartrate.
    qt::PlotWidget::SeriesBuffer getWindowBuffer() const;
    /// returns the framerate
    int getFramerate() const;
    /// returns the history depth
    int getHistoryDepth() const;

    /// Destructor
    virtual ~HeartrateDetector();

  };

  } // namespace icl::cv