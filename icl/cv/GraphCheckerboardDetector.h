// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/cv/CheckerboardDetector.h>
#include <icl/cv/CheckerboardSaddleDetector.h>

namespace icl{
  namespace cv{

    /// Native (OpenCV-free) CheckerboardDetector backend — ROCHADE-style graph topology.
    /** ChESS-style saddle/X-junction seeds (CheckerboardSaddleDetector) followed
        by graph-topology grid recovery (recoverCheckerboardGridGraph): Delaunay
        adjacency pruned to the true grid edges by image black/white-border
        evidence, then integer (col,row) assigned by BFS over that graph. Unlike
        NativeCheckerboardDetector (greedy growth) and RansacCheckerboardDetector
        (global geometric model), it never bootstraps a single local axis pair from
        lengths, so it is robust to the "diagonal trap" under steep oblique views.
        REQUIRES the image (the edge evidence is what removes the cell diagonals);
        discovers the lattice on its own — the board-dimensions hint is ignored.

        EVALUATION (Session 90): on COMPLETE lattices this is the most robust
        backend — it recovers the full grid through the 45deg oblique shear that
        traps greedy growth. But on real, PARTIAL frames (missing corners + a few
        spurious seeds) the topological BFS shears: a hole breaks the 4-connectivity
        the coordinate walk relies on, and the cleanup pass cannot recover a sheared
        labelling. So it is NOT yet competitive with native-growth / native-ransac on
        real data and is provided as an experimental backend + comparison-harness
        substrate, not a default. Making it partial-grid-robust (e.g. multi-seed
        consensus, or a global model re-fit instead of a pure walk) is future work. */
    class ICLCV_API GraphCheckerboardDetector : public CheckerboardDetector{
      CheckerboardSaddleDetector::Params m_saddle;
      bool m_cleanup = false;   ///< run the homography + Hungarian cleanup pass
      bool m_subpixel = true;   ///< run the final gradient sub-pixel corner polish

      public:
      GraphCheckerboardDetector() = default;
      explicit GraphCheckerboardDetector(const CheckerboardSaddleDetector::Params &p)
        : m_saddle(p) {}

      const CheckerboardSaddleDetector::Params &getSaddleParams() const { return m_saddle; }
      void setSaddleParams(const CheckerboardSaddleDetector::Params &p) { m_saddle = p; }

      /// Enable the global cleanup pass (refineCheckerboardGrid): robust
      /// homography + Hungarian re-association + boundary trim.
      void setCleanup(bool on) { m_cleanup = on; }
      bool getCleanup() const { return m_cleanup; }

      /// Enable the final sub-pixel corner polish (refineCheckerboardCornersSubPix):
      /// gradient-based saddle refinement on the grayscale image. On by default.
      void setSubPixel(bool on) { m_subpixel = on; }
      bool getSubPixel() const { return m_subpixel; }

      Result detect(const core::Img8u &image, const Hints &hints = {}) override;
      std::string name() const override { return m_cleanup ? "native-graph+lap" : "native-graph"; }
    };

  } // namespace cv
} // namespace icl
