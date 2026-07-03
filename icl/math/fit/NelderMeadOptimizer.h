// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/math/fit/Optimizer.h>
#include <icl/math/fit/SimplexOptimizer.h>

namespace icl::math {

  /// Nelder-Mead downhill-simplex optimizer behind the Optimizer<V> interface.
  /** Thin Configurable wrapper over SimplexOptimizer; exposes the iteration cap
      and the two stopping thresholds as properties. Serves as the reference
      implementation of the Tier-B Optimizer interface (a CMA-ES sibling can drop
      in later without touching call sites). */
  template<class V>
  class NelderMeadOptimizer : public Optimizer<V> {
    public:
    using Scalar    = typename Optimizer<V>::Scalar;
    using Objective = typename Optimizer<V>::Objective;
    using Result    = typename Optimizer<V>::Result;

    NelderMeadOptimizer(int maxIterations = 100000,
                        double minError = 1e-10,
                        double minDelta = 1e-10){
      using namespace utils;
      this->addProperty("max iterations", prop::Range<int>{.min=1, .max=1000000, .step=1},
                        maxIterations, "iteration cap");
      this->addProperty("min error", prop::Range<float>{.min=0.0, .max=1.0},
                        minError, "objective value at which to stop");
      this->addProperty("min delta", prop::Range<float>{.min=0.0, .max=1.0},
                        minDelta, "minimal simplex movement before stopping");
    }

    Result minimize(const Objective &f, const V &init) override {
      const int    maxIt    = this->getPropertyValue("max iterations");
      const double minError = this->getPropertyValue("min error");
      const double minDelta = this->getPropertyValue("min delta");

      SimplexOptimizer<Scalar,V> opt(f, int(VectorTraits<V>::dim(init)),
                                     maxIt, Scalar(minError), Scalar(minDelta));
      auto r = opt.optimize(init);
      return Result{ r.x, r.fx, r.iterations, double(r.fx) <= minError };
    }
  };

} // namespace icl::math
