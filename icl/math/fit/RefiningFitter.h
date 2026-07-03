// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/config/Configurable.h>
#include <icl/math/fit/ModelFitter.h>
#include <vector>

namespace icl::math {

  /// Refines an existing model estimate (Tier-C companion to ModelFitter).
  /** A RefiningFitter improves a seed model given the data — e.g. an LM /
      Nelder-Mead geometric-distance polish over an algebraic seed. Unlike
      ModelFitter, refine() takes the current estimate as input. Configurable so
      its tunables surface uniformly. */
  template<class Data, class Model>
  class RefiningFitter : public utils::Configurable {
    public:
    virtual ~RefiningFitter() = default;
    /// return an improved model given the data and a seed estimate
    virtual Model refine(const std::vector<Data> &data, const Model &seed) = 0;
  };

  /// Chains a seed fitter into a refiner — the generic "algebraic → geometric" pipe.
  /** SeededFitter is a ModelFitter whose fit() runs a seed ModelFitter and hands
      the result to a RefiningFitter:  fit(data) = refiner.refine(data, seed.fit(data)).
      Because it is-a ModelFitter, the whole chain composes further (e.g. wrap it in
      a RobustFitter). Both stages are surfaced as child Configurables. */
  template<class Data, class Model>
  class SeededFitter : public ModelFitter<Data,Model> {
    ModelFitter<Data,Model>    *m_seed;
    RefiningFitter<Data,Model> *m_refiner;
    public:
    SeededFitter(ModelFitter<Data,Model> *seed, RefiningFitter<Data,Model> *refiner)
      : m_seed(seed), m_refiner(refiner){
      if(m_seed)    this->addChildConfigurable(m_seed,    "seed.");
      if(m_refiner) this->addChildConfigurable(m_refiner, "refiner.");
    }
    Model fit(const std::vector<Data> &data) override {
      return m_refiner->refine(data, m_seed->fit(data));
    }
    double residual(const Model &m, const Data &d) const override { return m_seed->residual(m,d); }
    int    minSamples() const override { return m_seed->minSamples(); }
  };

} // namespace icl::math
