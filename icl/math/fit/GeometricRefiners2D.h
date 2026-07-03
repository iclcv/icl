// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/prop/Constraints.h>
#include <icl/math/fit/RefiningFitter.h>
#include <icl/math/fit/NelderMeadOptimizer.h>
#include <icl/utils/Point.h>
#include <algorithm>
#include <cmath>

namespace icl::math {

  /// Geometric (orthogonal-distance) circle refiner.
  /** Minimises the true perpendicular error  Σ (‖p − c‖ − r)²  with Nelder-Mead,
      seeded from an algebraic circle estimate — the ML-optimal fit under isotropic
      noise, and the standard "algebraic seed → geometric polish" step. The Model
      is the same [a,b,c,d] coefficient vector as the algebraic circle fitters, so
      GeometricCircleRefiner drops straight into a SeededFitter. Demonstrates a
      Tier-B Optimizer driving a Tier-C refiner. */
  class GeometricCircleRefiner : public RefiningFitter<utils::Point32f, std::vector<double> > {
    public:
    using Model = std::vector<double>;

    GeometricCircleRefiner(int maxIterations = 2000){
      using namespace utils;
      this->addProperty("max iterations", prop::Range<int>{.min=1, .max=100000, .step=1},
                        maxIterations, "Nelder-Mead iteration cap");
    }

    Model refine(const std::vector<utils::Point32f> &data, const Model &seed) override {
      // seed [a,b,c,d] : a(x²+y²)+b x+c y+d = 0  →  (cx, cy, r)
      const double a  = seed[0];
      const double cx0 = -seed[1] / (2*a);
      const double cy0 = -seed[2] / (2*a);
      const double r0  = std::sqrt(std::max(0.0,
                          (seed[1]*seed[1] + seed[2]*seed[2]) / (4*a*a) - seed[3]/a));

      using V = std::vector<double>;
      auto cost = [&data](const V &p) -> double {
        double s = 0;
        for(const auto &pt : data){
          const double dx = pt.x - p[0], dy = pt.y - p[1];
          const double e  = std::sqrt(dx*dx + dy*dy) - p[2];
          s += e*e;
        }
        return s;
      };

      NelderMeadOptimizer<V> opt(int(this->getPropertyValue("max iterations")), 1e-14, 1e-14);
      const auto res = opt.minimize(cost, V{cx0, cy0, r0});
      const double cx = res.params[0], cy = res.params[1], r = std::abs(res.params[2]);
      return { 1.0, -2*cx, -2*cy, cx*cx + cy*cy - r*r };
    }
  };

} // namespace icl::math
