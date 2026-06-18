// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/core/Img.h>

namespace icl::physics2 {

  /// Discretized paper space recording the stiffness of every fold (a physics2
  /// internal; not installed). Transplanted from the legacy `physics::FoldMap`.
  /** Each pixel holds a paper-stiffness value. A link created on the paper takes
      the minimum stiffness intersected along it, so any link crossing a fold line
      is weak (the crease bends). Memorized folds are stored as *negative* values:
      a bending constraint gets a positive stiffness if it crosses a non-memorized
      fold, or — if it crosses only a memorized (<0) link — the memorization
      property. */
  class FoldMap {
    core::Img32f m;
    float initialValue;
    void draw_fold(const utils::Point32f &a, const utils::Point32f &b, float val);

  public:
    FoldMap(const utils::Size &resolution = utils::Size(200, 300), float initialValue = 1);
    explicit FoldMap(const core::Img32f &image, float initialValue = 1);

    void clear();

    /// memorized folds are negative
    void addFold(const utils::Point32f &a, const utils::Point32f &b, float value);

    /// sets fold pixels back to the initial value
    void removeFold(const utils::Point32f &a, const utils::Point32f &b);

    /// returns a value usable as fold stiffness (prefers non-1 positive values;
    /// only if the minimum is 1 does it return the maximum negative value)
    float getFoldValue(const utils::Point32f &a, const utils::Point32f &b);

    /// current fold map (for display)
    const core::Img32f &getDisplay() const { return m; }
  };

} // namespace icl::physics2
