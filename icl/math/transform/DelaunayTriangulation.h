// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Point.h>
#include <utility>
#include <vector>

namespace icl::math {

  /// One triangle of a Delaunay triangulation, as indices into the input list.
  /** Vertices are stored counter-clockwise. */
  struct DelaunayTriangle {
    int a, b, c;
  };

  /// Delaunay triangulation of a 2D point set (Bowyer-Watson, doubles internally).
  /** Returns the triangulation of \a points as index triples (counter-clockwise).
      The simple incremental form is O(n^2) worst case, which is ample for the
      few-hundred-point sets this is meant for (e.g. detected checkerboard
      corners). Fewer than three points, or fully collinear input, yields an empty
      result. Exactly-cocircular points are resolved arbitrarily but consistently
      (the triangulation stays valid). Duplicate points should be removed by the
      caller; a coincident point produces a degenerate (zero-area) triangle. */
  ICLMath_API std::vector<DelaunayTriangle>
  delaunayTriangulation(const std::vector<utils::Point32f> &points);

  /// Undirected edge graph of a triangulation: each shared point-index pair once.
  /** Convenience over delaunayTriangulation(): collapses the triangle list to the
      set of unique edges (a,b) with a<b — i.e. the corner-adjacency graph. */
  ICLMath_API std::vector<std::pair<int, int>>
  delaunayEdges(const std::vector<DelaunayTriangle> &triangles);

} // namespace icl::math
