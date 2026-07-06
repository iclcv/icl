// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/prop/Constraints.h>
#include <icl/utils/Random.h>
#include <icl/math/fit/Optimizer.h>
#include <icl/math/la/DynMatrix.h>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <vector>

namespace icl::math {

  /// CMA-ES (Covariance Matrix Adaptation Evolution Strategy) optimizer.
  /** A rank-based, derivative-free optimizer (Hansen) that learns the covariance
      of successful steps — the modern default for noisy, ill-conditioned or
      multimodal black-box objectives, and far more robust than Nelder-Mead
      restarts. Implements the Optimizer<V>
      interface, so it is a drop-in for NelderMeadOptimizer.

      Stochastic: seed the global RNG (`utils::randomSeed(...)`) for reproducibility.
      Properties: max iterations (generations), initial sigma (step size),
      min error (target), population λ (0 = auto 4+⌊3·ln N⌋). */
  template<class V>
  class CMAESOptimizer : public Optimizer<V> {
    public:
    using Scalar    = typename Optimizer<V>::Scalar;
    using Objective = typename Optimizer<V>::Objective;
    using Result    = typename Optimizer<V>::Result;

    CMAESOptimizer(int maxIterations = 1000, double sigma0 = 0.3,
                   double minError = 1e-12, int lambda = 0){
      using namespace utils;
      this->addProperty("max iterations", prop::Range<int>{.min=1, .max=1000000, .step=1},
                        maxIterations, "generation cap");
      this->addProperty("initial sigma", prop::Range<float>{.min=1e-6f, .max=1e6f},
                        float(sigma0), "initial step size (coordinate std)");
      this->addProperty("min error", prop::Range<float>{.min=0.f, .max=1.f},
                        float(minError), "target objective value to stop");
      this->addProperty("population", prop::Range<int>{.min=0, .max=100000, .step=1},
                        lambda, "offspring per generation λ (0 = auto)");
    }

    Result minimize(const Objective &f, const V &init) override {
      using M = DynMatrix<double>;
      const int    N        = int(VectorTraits<V>::dim(init));
      const int    maxIt    = this->getPropertyValue("max iterations");
      const double stopFit  = double(float(this->getPropertyValue("min error")));
      double       sigma    = double(float(this->getPropertyValue("initial sigma")));
      int          lambda   = this->getPropertyValue("population");
      if(lambda <= 0) lambda = 4 + int(std::floor(3.0*std::log(double(N))));
      const int    mu       = lambda/2;

      // recombination weights (sum to 1) and variance-effective mass
      std::vector<double> w(mu);
      for(int i=0;i<mu;++i) w[i] = std::log(mu+0.5) - std::log(double(i+1));
      double sw=0; for(double x:w) sw+=x; for(double &x:w) x/=sw;
      double sw2=0; for(double x:w) sw2+=x*x;
      const double mueff = 1.0/sw2;

      // strategy parameters (Hansen defaults)
      const double cc    = (4.0+mueff/N)/(N+4.0+2.0*mueff/N);
      const double cs    = (mueff+2.0)/(N+mueff+5.0);
      const double c1    = 2.0/((N+1.3)*(N+1.3)+mueff);
      const double cmu   = std::min(1.0-c1, 2.0*(mueff-2.0+1.0/mueff)/((N+2.0)*(N+2.0)+mueff));
      const double damps = 1.0 + 2.0*std::max(0.0, std::sqrt((mueff-1.0)/(N+1.0))-1.0) + cs;
      const double chiN  = std::sqrt(double(N))*(1.0 - 1.0/(4.0*N) + 1.0/(21.0*N*N));

      auto ident = [&](){ M m = M::create(N,N,0.0); for(int i=0;i<N;++i) m(i,i)=1.0; return m; };

      M xmean(N,1); for(int i=0;i<N;++i) xmean[i] = double(VectorTraits<V>::get(init,i));
      M pc = M::create(N,1,0.0), ps = M::create(N,1,0.0);
      M B = ident(), C = ident(), invsqrtC = ident();
      std::vector<double> D(N, 1.0);

      auto toV = [&](const M &x){ V v = init; for(int i=0;i<N;++i) VectorTraits<V>::set(v,i,Scalar(x[i])); return v; };

      utils::GRand gauss(0.0, 1.0);
      V      bestParams = init;
      double bestFit    = f(init);
      int    it = 0;

      std::vector<M>      arx(lambda, M(N,1));
      std::vector<double> fit(lambda);
      std::vector<int>    idx(lambda);

      for(it=0; it<maxIt; ++it){
        // ---- sample & evaluate offspring ----
        for(int k=0;k<lambda;++k){
          M z(N,1); for(int i=0;i<N;++i) z[i] = double(gauss);
          M Dz(N,1); for(int i=0;i<N;++i) Dz[i] = D[i]*z[i];
          const M y = B*Dz;                                  // ~ N(0,C)
          M x(N,1); for(int i=0;i<N;++i) x[i] = xmean[i] + sigma*y[i];
          arx[k] = x;
          fit[k] = f(toV(x));
        }
        std::iota(idx.begin(), idx.end(), 0);
        std::sort(idx.begin(), idx.end(), [&](int a,int b){ return fit[a] < fit[b]; });

        if(fit[idx[0]] < bestFit){ bestFit = fit[idx[0]]; bestParams = toV(arx[idx[0]]); }

        // ---- recombination: new mean ----
        const M xold = xmean;
        for(int i=0;i<N;++i){ double s=0; for(int j=0;j<mu;++j) s += w[j]*arx[idx[j]][i]; xmean[i]=s; }
        M ymean(N,1); for(int i=0;i<N;++i) ymean[i] = (xmean[i]-xold[i])/sigma;

        // ---- evolution paths ----
        const M Cy = invsqrtC*ymean;
        const double aps = std::sqrt(cs*(2.0-cs)*mueff);
        for(int i=0;i<N;++i) ps[i] = (1.0-cs)*ps[i] + aps*Cy[i];
        const double psNorm = ps.norm();
        const bool hsig = psNorm/std::sqrt(1.0-std::pow(1.0-cs, 2.0*(it+1)))/chiN
                          < (1.4 + 2.0/(N+1));
        const double apc = hsig ? std::sqrt(cc*(2.0-cc)*mueff) : 0.0;
        for(int i=0;i<N;++i) pc[i] = (1.0-cc)*pc[i] + apc*ymean[i];

        // ---- covariance update (rank-1 + rank-mu) ----
        const double ch = (hsig ? 0.0 : 1.0)*cc*(2.0-cc);
        M Cnew = M::create(N,N,0.0);
        for(int r=0;r<N;++r) for(int c=0;c<N;++c){
          double rankMu = 0;
          for(int j=0;j<mu;++j){
            const double yr = (arx[idx[j]][r]-xold[r])/sigma;
            const double yc = (arx[idx[j]][c]-xold[c])/sigma;
            rankMu += w[j]*yr*yc;
          }
          Cnew(r,c) = (1.0-c1-cmu)*C(r,c) + c1*(pc[r]*pc[c] + ch*C(r,c)) + cmu*rankMu;
        }
        C = Cnew;

        // ---- step-size control ----
        sigma *= std::exp((cs/damps)*(psNorm/chiN - 1.0));

        // ---- decompose C -> B, D, invsqrtC ----
        for(int r=0;r<N;++r) for(int c=r+1;c<N;++c){ const double a=0.5*(C(r,c)+C(c,r)); C(r,c)=C(c,r)=a; }
        auto [evec, eval] = C.eigen();
        B = evec;
        for(int i=0;i<N;++i) D[i] = std::sqrt(std::max(0.0, eval[i]));
        M Dinv = M::create(N,N,0.0);
        for(int i=0;i<N;++i) Dinv(i,i) = (D[i] > 1e-20) ? 1.0/D[i] : 0.0;
        invsqrtC = B*Dinv*B.transp();

        if(bestFit < stopFit) break;
        if(sigma < 1e-15 || !std::isfinite(sigma)) break;   // collapsed / diverged
      }

      return Result{ bestParams, Scalar(bestFit), it, bestFit < stopFit };
    }
  };

} // namespace icl::math
