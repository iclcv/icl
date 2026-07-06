// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <algorithm>
#include <cmath>
#include <vector>

namespace icl::math {

  /// M-estimator robust-loss kernel for Iteratively Reweighted Least Squares.
  /** A robust kernel down-weights large residuals so a least-squares fit tolerates
      outliers, instead of a single gross outlier dragging the whole fit. In IRLS,
      each residual r gets a weight w(r) = ψ(r)/r (ψ = ρ', the loss derivative);
      the weighted normal equations are then re-solved until convergence.

      Residuals are normalised by a robust scale σ (typically MAD-estimated, see
      madScale) times a per-kernel tuning constant c chosen for 95% efficiency
      under Gaussian noise. `None` reproduces plain (non-robust) least squares. */
  struct RobustKernel {
    enum Type { None, Huber, Cauchy, Tukey };
    Type type = None;
    double c = 0.0;   //!< tuning constant; 0 → the per-kernel 95%-efficiency default

    /// tuning constant (explicit c if >0, else the kernel's default)
    double tuning() const {
      if(c > 0.0) return c;
      switch(type){
        case Huber:  return 1.345;
        case Cauchy: return 2.3849;
        case Tukey:  return 4.685;
        default:     return 1.0;
      }
    }

    /// IRLS weight for a (non-negative) residual r at robust scale sigma.
    double weight(double r, double sigma) const {
      if(type == None || sigma <= 0.0) return 1.0;
      const double u = r / (tuning() * sigma);
      switch(type){
        case Huber:  return (u <= 1.0) ? 1.0 : 1.0/u;
        case Cauchy: return 1.0/(1.0 + u*u);
        case Tukey:  return (u <= 1.0) ? (1.0-u*u)*(1.0-u*u) : 0.0;
        default:     return 1.0;
      }
    }

    /// robust scale estimate σ = 1.4826 · median(|r_i|) (MAD, residuals ~0-centred).
    static double madScale(std::vector<double> absResiduals){
      if(absResiduals.empty()) return 0.0;
      const size_t m = absResiduals.size()/2;
      std::nth_element(absResiduals.begin(), absResiduals.begin()+m, absResiduals.end());
      double med = absResiduals[m];
      if(absResiduals.size()%2 == 0){
        std::nth_element(absResiduals.begin(), absResiduals.begin()+m-1, absResiduals.end());
        med = 0.5*(med + absResiduals[m-1]);
      }
      return 1.4826 * med;
    }
  };

} // namespace icl::math
