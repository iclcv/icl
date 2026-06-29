// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/cv/CheckerboardDetector.h>
#include <icl/cv/CheckerboardSaddleDetector.h>

namespace icl{
  namespace cv{

    /// Native (OpenCV-free) CheckerboardDetector backend.
    /** ChESS-style saddle/X-junction seeds (CheckerboardSaddleDetector) followed
        by growth-based, distortion-tolerant grid recovery
        (recoverCheckerboardGrid). Discovers the lattice on its own — the board
        dimensions hint is not required (and is currently ignored). */
    class ICLCV_API NativeCheckerboardDetector : public CheckerboardDetector{
      CheckerboardSaddleDetector::Params m_saddle;
      bool m_cleanup = false;   ///< run the homography + Hungarian cleanup pass

      public:
      NativeCheckerboardDetector() = default;
      explicit NativeCheckerboardDetector(const CheckerboardSaddleDetector::Params &p)
        : m_saddle(p) {}

      const CheckerboardSaddleDetector::Params &getSaddleParams() const { return m_saddle; }
      void setSaddleParams(const CheckerboardSaddleDetector::Params &p) { m_saddle = p; }

      /// Enable the global cleanup pass (refineCheckerboardGrid): robust
      /// homography + Hungarian re-association + boundary trim, to suppress
      /// spurious border detections. Off by default — a plain homography
      /// mispredicts strongly lens-distorted borders, so this is opt-in until a
      /// distortion estimate can be fed back through Hints.
      void setCleanup(bool on) { m_cleanup = on; }
      bool getCleanup() const { return m_cleanup; }

      Result detect(const core::Img8u &image, const Hints &hints = {}) override;
      std::string name() const override { return m_cleanup ? "native-growth+lap" : "native-growth"; }
    };

  } // namespace cv
} // namespace icl
