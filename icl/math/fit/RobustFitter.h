// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Random.h>
#include <icl/math/fit/ModelFitter.h>
#include <icl/math/fit/FitUtils.h>
#include <limits>
#include <string>

namespace icl::math {

  /// Robust model fitting (RANSAC / MSAC + local optimization) as a decorator.
  /** RobustFitter is a ModelFitter that wraps another ModelFitter (the "base")
      and makes it outlier-tolerant. It repeatedly fits the base to a random
      minimal sample, scores each hypothesis against all data, and keeps the best;
      the iteration budget adapts to the best inlier ratio (adaptiveRansacIters).

      Because it is-a ModelFitter, it composes: `RobustFitter(&circleFitter)` is a
      robust circle fitter, and it can itself be wrapped or chained.

      Scoring (property "scoring"):
      - "msac"  : truncated-quadratic cost  Σ min(r², t²)  — MLESAC-style, the
                  default; strictly dominates plain inlier counting at no cost.
      - "ransac": classic inlier count (cost = number of outliers).

      Local optimization (property "local optimization", LO-RANSAC): when enabled,
      the winning model is refit on its full inlier set and re-scored, iterated
      until the inlier set stops growing. Large accuracy gain for little cost.

      The base fitter is added as a child Configurable ("base." prefix), so its
      tunables surface alongside the robust parameters. */
  template<class Data, class Model>
  class RobustFitter : public ModelFitter<Data,Model> {
    ModelFitter<Data,Model> *m_base;
    std::vector<Data> m_inliers;

    public:
    RobustFitter(ModelFitter<Data,Model> *base,
                 double inlierThreshold,
                 double confidence = 0.99,
                 int maxIterations = 1000,
                 bool msac = true,
                 bool localOpt = true)
      : m_base(base){
      using namespace utils;
      this->addProperty("inlier threshold", prop::Range<float>{.min=0.0, .max=1e9},
                        inlierThreshold, "max residual for a point to count as inlier");
      this->addProperty("confidence", prop::Range<float>{.min=0.5, .max=0.999999},
                        confidence, "target probability of drawing a clean sample");
      this->addProperty("max iterations", prop::Range<int>{.min=1, .max=1000000, .step=1},
                        maxIterations, "hard cap on RANSAC iterations");
      this->addProperty("scoring", prop::Menu<std::string>{"msac", "ransac"},
                        std::string(msac ? "msac" : "ransac"), "hypothesis scoring");
      this->addProperty("local optimization", prop::Flag{},
                        localOpt, "LO-RANSAC: refit the winner on its inliers");
      if(m_base) this->addChildConfigurable(m_base, "base.");
    }

    int    minSamples() const override { return m_base->minSamples(); }
    double residual(const Model &m, const Data &d) const override { return m_base->residual(m,d); }

    /// inliers of the last fit() (empty if no model was found)
    const std::vector<Data>& inliers() const { return m_inliers; }

    Model fit(const std::vector<Data> &data) override {
      const int    s      = m_base->minSamples();
      const double thresh = this->getPropertyValue("inlier threshold");
      const double conf   = this->getPropertyValue("confidence");
      const int    maxIt  = this->getPropertyValue("max iterations");
      const std::string scoring = this->getPropertyValue("scoring");
      const bool   msac   = (scoring == "msac");
      const bool   lo     = this->getPropertyValue("local optimization");

      m_inliers.clear();
      const int N = int(data.size());
      if(N < s) return Model();

      const double t2 = thresh * thresh;
      std::vector<Data> sample(s);
      std::vector<int>  idx(s);

      Model  bestModel;
      double bestCost = std::numeric_limits<double>::max();
      bool   found = false;
      int    nReq = maxIt;

      for(int i = 0; i < maxIt && i < nReq; ++i){
        utils::get_random_subset(data, s, sample, idx);
        Model m;
        try { m = m_base->fit(sample); } catch(...) { continue; }

        double cost = 0.0; int nin = 0;
        for(const Data &d : data){
          const double r = m_base->residual(m, d);
          if(r < thresh){ ++nin; cost += msac ? r*r : 0.0; }
          else           cost += msac ? t2 : 1.0;   // ransac: count outliers
        }
        if(cost < bestCost){
          bestCost = cost; bestModel = m; found = true;
          const int est = adaptiveRansacIters(double(nin) / N, s, conf);
          if(est < nReq) nReq = est;
        }
      }
      if(!found) return Model();

      m_inliers = collectInliers(bestModel, data, thresh);
      if(lo){
        for(int k = 0; k < 10; ++k){
          if(int(m_inliers.size()) < s) break;
          Model refined;
          try { refined = m_base->fit(m_inliers); } catch(...) { break; }
          std::vector<Data> in2 = collectInliers(refined, data, thresh);
          const bool grew = in2.size() > m_inliers.size();
          bestModel = refined; m_inliers = in2;
          if(!grew) break;                       // converged
        }
      }
      return bestModel;
    }

    private:
    std::vector<Data> collectInliers(const Model &m, const std::vector<Data> &data,
                                     double thresh) const {
      std::vector<Data> in; in.reserve(data.size());
      for(const Data &d : data) if(m_base->residual(m, d) < thresh) in.push_back(d);
      return in;
    }
  };

} // namespace icl::math
