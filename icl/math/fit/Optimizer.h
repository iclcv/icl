// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/config/Configurable.h>
#include <icl/utils/prop/Constraints.h>
#include <icl/math/fit/VectorTraits.h>
#include <functional>

namespace icl::math {

  /// Generic derivative-free optimizer interface (Tier B of the fit framework).
  /** An Optimizer minimises a scalar objective f: V -> Scalar over a parameter
      vector of type V (std::vector<S>, DynColVector<S> or FixedColVector<S,D>, via
      VectorTraits). Concrete implementations — Nelder-Mead, (later) CMA-ES,
      (1+1)-ES — are interchangeable behind this interface, and Configurable so
      their tunables (iteration cap, stopping thresholds, ...) surface uniformly.
      minimize() is non-const: optimizers are stateful. */
  template<class V>
  class Optimizer : public utils::Configurable {
    public:
    using Scalar    = typename VectorTraits<V>::Scalar;
    using Objective = std::function<Scalar(const V&)>;
    using InitGen   = std::function<V()>;   //!< produces a (random) start point

    /// optimization outcome
    struct Result {
      V      params;      //!< best parameter vector found
      Scalar error;       //!< objective value at params
      int    iterations;  //!< iterations performed
      bool   converged;   //!< true if a stopping criterion (not the cap) was hit
    };

    virtual ~Optimizer() = default;

    /// minimise f starting from init
    virtual Result minimize(const Objective &f, const V &init) = 0;

    /// Multi-start minimisation: run minimize() from nStarts independent starts
    /// (each drawn from initGen) and keep the best — the standard remedy for
    /// local minima. Generic over any concrete Optimizer (built on minimize()).
    Result minimizeRestarts(const Objective &f, const InitGen &initGen, int nStarts){
      Result best{}; bool have = false;
      for(int i=0;i<nStarts;++i){
        const Result r = minimize(f, initGen());
        if(!have || r.error < best.error){ best = r; have = true; }
      }
      return best;
    }
  };

} // namespace icl::math
