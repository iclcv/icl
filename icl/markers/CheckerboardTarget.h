// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/markers/CalibrationTarget.h>
#include <memory>

namespace icl::cv { class CheckerboardDetector; }

namespace icl::markers {

  /// Checkerboard calibration target (native, OpenCV-free).
  /** Detects with cv::CheckerboardSaddleDetector (sub-pixel X-junctions) +
      growth-based grid recovery (cv::recoverCheckerboardGrid), then labels the
      recovered lattice against the known board geometry to emit object↔image
      correspondences. \a cols × \a rows count the CHECKER SQUARES; the detectable
      inner corners are (cols-1) × (rows-1).

      v1 requires the full inner lattice to be recovered (dimensions matching the
      board, in either axis order — the recovered origin/orientation is otherwise
      arbitrary, which is fine for per-view calibration). Partial detection and
      absolute board-frame disambiguation are future work. */
  class ICLMarkers_API CheckerboardTarget : public CalibrationTarget {
    int   m_cols;        ///< checker squares along x
    int   m_rows;        ///< checker squares along y
    float m_squareMM;    ///< square edge length [mm]
    std::shared_ptr<cv::CheckerboardDetector> m_detector;  ///< pixels→lattice technique

  public:
    /// \a cols × \a rows checker squares, each \a squareSizeMM wide. Uses the
    /// native ChESS-saddle + growth detector unless overridden via setDetector().
    CheckerboardTarget(int cols = 8, int rows = 6, float squareSizeMM = 25.f);

    /// Swap the underlying detection technique (e.g. native-growth vs opencv).
    void setDetector(std::shared_ptr<cv::CheckerboardDetector> detector);
    /// The active detection technique.
    std::shared_ptr<cv::CheckerboardDetector> getDetector() const { return m_detector; }

    std::vector<CalibrationCorrespondence> detect(const core::Img8u &image) const override;
    std::vector<cv3d::Vec> modelPoints() const override;
    core::Img8u generate(const utils::Size &pixelSize) const override;
    std::string name() const override { return "checkerboard"; }

    int   getCols()       const { return m_cols; }
    int   getRows()       const { return m_rows; }
    float getSquareSize() const { return m_squareMM; }
  };

} // namespace icl::markers
