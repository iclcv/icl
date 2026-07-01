// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/cv/CheckerboardDetector.h>
#include <icl/cv/CheckerboardSaddleDetector.h>

namespace icl{
  namespace cv{

    /// Native (OpenCV-free) CheckerboardDetector backend — global RANSAC association.
    /** ChESS-style saddle/X-junction seeds (CheckerboardSaddleDetector) followed
        by foreshortening-robust GLOBAL grid recovery
        (recoverCheckerboardGridRansac): a RANSAC over affine axis hypotheses
        (scored by integer-lattice inlier count) → homography ICP → compactness
        de-shear. Unlike NativeCheckerboardDetector's greedy growth, it never
        bootstraps a single local axis pair, so it does not fall into the
        "diagonal trap" under steep oblique views. Discovers the lattice on its
        own — the board-dimensions hint is not required (and is ignored). */
    class ICLCV_API RansacCheckerboardDetector : public CheckerboardDetector{
      CheckerboardSaddleDetector::Params m_saddle;
      bool m_cleanup = false;   ///< run the homography + Hungarian cleanup pass
      bool m_subpixel = true;   ///< run the final gradient sub-pixel corner polish

      public:
      RansacCheckerboardDetector() = default;
      explicit RansacCheckerboardDetector(const CheckerboardSaddleDetector::Params &p)
        : m_saddle(p) {}

      const CheckerboardSaddleDetector::Params &getSaddleParams() const { return m_saddle; }
      void setSaddleParams(const CheckerboardSaddleDetector::Params &p) { m_saddle = p; }

      /// Enable the global cleanup pass (refineCheckerboardGrid): robust
      /// homography + Hungarian re-association + boundary trim, to suppress
      /// spurious border detections.
      void setCleanup(bool on) { m_cleanup = on; }
      bool getCleanup() const { return m_cleanup; }

      /// Enable the final sub-pixel corner polish (refineCheckerboardCornersSubPix):
      /// gradient-based saddle refinement on the grayscale image. On by default.
      void setSubPixel(bool on) { m_subpixel = on; }
      bool getSubPixel() const { return m_subpixel; }

      Result detect(const core::Img8u &image, const Hints &hints = {}) override;
      std::string name() const override { return m_cleanup ? "native-ransac+lap" : "native-ransac"; }
    };

  } // namespace cv
} // namespace icl
