// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/cv/CheckerboardGrid.h>
#include <icl/core/Img.h>
#include <icl/utils/Size.h>
#include <string>
#include <vector>

namespace icl {
  namespace filter { class ImageUndistortion; }

  namespace cv {

    /// Abstract interface for checkerboard detection techniques.
    /** A detector turns a raw image into one or more ordered checkerboard
        lattices (cv::CheckerboardGrid). This is the swappable seam under
        markers::CheckerboardTarget: the native ChESS-saddle + growth pipeline
        (NativeCheckerboardDetector) and the OpenCV findChessboardCorners wrapper
        (OpenCVCheckerboardDetector) are two backends behind one detect() call,
        which is what the calibration comparison harness drives.

        Hints carry optional side information a technique may exploit: the known
        board dimensions (some backends, e.g. OpenCV, REQUIRE them) and the
        current lens-distortion estimate (the feedback channel for an iterative
        undistortion bootstrap — straighten the lattice to reach border boards).
        A backend is free to ignore any hint it does not use. */
    class CheckerboardDetector {
      public:
      /// Optional side information for a detection pass.
      struct Hints {
        /// Known INNER-corner lattice dimensions (cols, rows). (0,0) means
        /// "discover" — only some backends can do that (the native growth one);
        /// fixed-template backends (OpenCV) require a non-empty size.
        utils::Size boardCells;
        /// Current lens-distortion estimate, or null. A distortion-aware backend
        /// may use it to predict/straighten the lattice and so reach boards near
        /// the image border (where distortion is strongest). Non-owning.
        const filter::ImageUndistortion *undistortion;
        // user-provided default ctor (not DMIs) so `Hints{}` works as a default
        // argument inside this enclosing class definition
        Hints() : boardCells(0, 0), undistortion(nullptr) {}
      };

      /// Detection result: zero or more recovered lattices.
      struct Result {
        std::vector<CheckerboardGrid> boards;
        bool empty() const { return boards.empty(); }
      };

      virtual ~CheckerboardDetector() = default;

      /// Detect checkerboard lattice(s) in \a image, optionally guided by \a hints.
      /** Non-const on purpose: backends may reuse internal buffers across calls. */
      virtual Result detect(const core::Img8u &image, const Hints &hints = {}) = 0;

      /// Short technique identifier (e.g. "native-growth", "opencv").
      virtual std::string name() const = 0;
    };

  } // namespace cv
} // namespace icl
