// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/config/Configurable.h>
#include <icl/utils/prop/Constraints.h>
#include <vector>

namespace icl::math {

  /// Universal "fit a model to data" contract (Tier C of the fit framework).
  /** A ModelFitter maps a set of Data points to a Model. The Data and Model types
      are opaque (default-constructible + copyable is all that is required), so the
      same interface serves algebraic 2D-primitive fits, homographies, poses, etc.

      The three virtuals are exactly what the generic robustifiers and pipelines
      need:
      - fit()        : estimate a model from a (minimal or full) point set
      - residual()   : per-point geometric/algebraic error wrt a model (>= 0)
      - minSamples() : minimal points to instantiate a model

      Because RobustFitter (RANSAC/MSAC + LO) is itself a ModelFitter that wraps a
      ModelFitter, fitters compose and chain generically. ModelFitter is a
      utils::Configurable, so a concrete fitter's tunables surface uniformly (and
      auto-render in qt::Prop). fit() is non-const: fitters are stateful and may
      reuse internal buffers across calls. */
  template<class Data, class Model>
  class ModelFitter : public utils::Configurable {
    public:
    virtual ~ModelFitter() = default;

    /// estimate a model from the given points
    virtual Model fit(const std::vector<Data> &data) = 0;

    /// non-negative residual of a single datum with respect to a model
    virtual double residual(const Model &model, const Data &d) const = 0;

    /// minimal number of points required to instantiate a model
    virtual int minSamples() const = 0;
  };

} // namespace icl::math
