// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/cv/CheckerboardSaddleDetector.h>
#include <icl/utils/Point.h>
#include <vector>

namespace icl::cv {

  /// A recovered checkerboard lattice: detected inner corners arranged on a
  /// rectangular integer (col,row) grid. Individual cells may be missing (not
  /// detected); use has()/at() to test/read them.
  struct CheckerboardGrid {
    int cols = 0;   ///< number of columns (label range [0,cols))
    int rows = 0;   ///< number of rows    (label range [0,rows))
    int count = 0;  ///< number of cells actually filled
    std::vector<utils::Point32f> points;   ///< cols*rows, row-major image positions
    std::vector<char> filled;              ///< cols*rows, 1 if that cell was detected

    bool empty() const { return count == 0; }
    bool has(int c, int r) const {
      return c >= 0 && r >= 0 && c < cols && r < rows && filled[(size_t)r*cols + c];
    }
    const utils::Point32f &at(int c, int r) const { return points[(size_t)r*cols + c]; }
    /// true if every cell of the cols*rows lattice is filled (a complete board)
    bool complete() const { return count == cols*rows && cols > 0; }
  };

  /// Recover a checkerboard lattice from saddle corner seeds (growth-based, v1).
  /** Topology comes from the seed geometry alone (the seeds' orientations seed
      the initial grid axes): per-seed nearest-neighbour spacing, then a BFS that
      grows an integer lattice by following the four local axis directions,
      propagating the measured local step vectors so it tracks perspective and
      lens distortion. The lattice grown from the highest-scoring seed is
      returned (its largest connected component); clutter and a few missing
      corners are tolerated. Returns an empty grid when there are too few seeds.

      The (col,row) origin and axis assignment are NOT canonicalised to any
      physical board frame — the labelling is internally consistent but otherwise
      arbitrary (good enough for intrinsic calibration; a target backend resolves
      the board frame). */
  ICLCV_API CheckerboardGrid recoverCheckerboardGrid(const std::vector<CornerSeed> &seeds);

} // namespace icl::cv
