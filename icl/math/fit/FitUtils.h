// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/math/la/DynMatrix.h>
#include <icl/math/la/DynVector.h>
#include <algorithm>
#include <cmath>
#include <limits>

namespace icl::math {

  /// Homogeneous null-space solve shared by the DLT / algebraic model fitters.
  /** Given a symmetric positive-semidefinite scatter matrix S = AᵀA, returns the
      eigenvector of its SMALLEST eigenvalue — the unit vector x minimising
      |A x|² subject to |x| = 1. This is the core step of Homography2D's DLT and
      of LeastSquareModelFitting's identity-constraint fit. Forming S already
      squares the condition number, so callers that can afford it should normalise
      their data first (see HartleyNormalization, deferred). The min-eigenvalue
      index is scanned explicitly so the result is correct regardless of the
      backend eigenvalue ordering. */
  template<class S>
  DynColVector<S> homogeneousNullSpace(const DynMatrix<S> &scatter){
    auto [evec, eval] = scatter.eigen();       // symmetric ⇒ real eigenpairs
    int mi = 0;
    for(unsigned int i = 1; i < eval.rows(); ++i) if(eval[i] < eval[mi]) mi = int(i);
    DynColVector<S> x(scatter.cols());
    std::copy(evec.col_begin(mi), evec.col_end(mi), x.begin());
    return x;
  }

  /// RANSAC adaptive iteration count.
  /** Number of minimal samples needed to draw an all-inlier sample with
      probability `confidence`, given an inlier ratio `w` and minimal sample size
      `s`: N = log(1-P) / log(1 - wˢ). Returns INT_MAX when w ≤ 0 (no information)
      and 1 when w ≥ 1 (all inliers). */
  inline int adaptiveRansacIters(double w, int s, double confidence = 0.99){
    if(w <= 0.0) return std::numeric_limits<int>::max();
    if(w >= 1.0) return 1;
    const double denom = std::log(1.0 - std::pow(w, s));
    if(denom >= 0.0) return std::numeric_limits<int>::max();
    return int(std::log(1.0 - confidence) / denom) + 1;
  }

} // namespace icl::math
