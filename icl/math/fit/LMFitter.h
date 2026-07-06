// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/math/fit/ModelFitter.h>
#include <icl/math/fit/LevenbergMarquardtFitter.h>
#include <icl/math/fit/RobustKernel.h>
#include <utility>

namespace icl::math {

  /// ModelFitter that fits a non-linear model y = f(β, x) by Levenberg-Marquardt,
  /// with an optional robust M-estimator kernel (IRLS).
  /** Folds the standalone LevenbergMarquardtFitter onto the fit-framework
      ModelFitter<Data,Model> contract so it composes with the rest of the
      framework (Configurable tunables, RobustFitter, pipelines):
      - Data  = (x, y) sample: std::pair<Vector,Vector>
      - Model = parameter vector β (Vector)

      When a RobustKernel other than None is set, the fit is Iteratively Reweighted:
      LM is re-run with per-sample weights w_i = kernel(r_i/σ) (σ MAD-estimated)
      until the parameters settle — so a handful of gross outliers no longer drag
      the whole fit (the classic robust-LM pairing, complementary to the
      RANSAC-style RobustFitter). Weights enter LM without touching its core by
      folding √w into each residual (√w·y vs √w·f). */
  template<class Scalar = float>
  class LMFitter : public ModelFitter< std::pair<DynColVector<Scalar>, DynColVector<Scalar> >,
                                       DynColVector<Scalar> > {
    public:
    using Vector   = DynColVector<Scalar>;
    using Mat      = DynMatrix<Scalar>;
    using Sample   = std::pair<Vector,Vector>;   //!< (x, y)
    using Model    = Vector;                      //!< parameter vector β
    using Function = typename LevenbergMarquardtFitter<Scalar>::Function;

    /// @param f          model function  y = f(β, x)
    /// @param inputDim   dimension of x
    /// @param outputDim  dimension of y
    /// @param start      initial parameter vector (also defines the param count)
    /// @param kernel     robust M-estimator (default None = plain least squares)
    /// @param irlsSteps  max IRLS reweighting rounds when a robust kernel is set
    LMFitter(Function f, int inputDim, int outputDim, const Vector &start,
             RobustKernel kernel = {}, int irlsSteps = 6)
      : m_f(f), m_inDim(inputDim), m_outDim(outputDim), m_start(start),
        m_kernel(kernel), m_irlsSteps(irlsSteps) {}

    /// number of samples needed to determine the model (= parameter count)
    int minSamples() const override { return (int)m_start.dim(); }

    /// residual of one sample wrt a model: ||y - f(β, x)||
    double residual(const Model &beta, const Sample &s) const override {
      const Vector e = s.second - m_f(beta, s.first);
      double a = 0; for(unsigned i=0;i<e.dim();++i) a += double(e[i])*e[i];
      return std::sqrt(a);
    }

    /// fit β to the samples (IRLS when a robust kernel is set)
    Model fit(const std::vector<Sample> &data) override {
      const int N = (int)data.size();
      if(!N) return m_start;

      Vector beta = m_start;
      std::vector<Scalar> w(N, Scalar(1));

      const int rounds = (m_kernel.type == RobustKernel::None) ? 1 : (m_irlsSteps+1);
      for(int round=0; round<rounds; ++round){
        // fold √w into x (extra column) and y so LM minimises Σ w_i·||y-f||²
        Mat xs = Mat::create(N, m_inDim+1), ys = Mat::create(N, m_outDim);
        for(int i=0;i<N;++i){
          const Scalar sw = std::sqrt(w[i]);
          for(int k=0;k<m_inDim;++k)  xs(i,k) = data[i].first[k];
          xs(i,m_inDim) = sw;                                   // weight ride-along
          for(int k=0;k<m_outDim;++k) ys(i,k) = sw*data[i].second[k];
        }
        Function f = m_f; const int inDim = m_inDim;
        Function fw = [f,inDim](const Vector &p, const Vector &xaug)->Vector{
          Vector x(inDim); for(int k=0;k<inDim;++k) x[k]=xaug[k];
          Vector y = f(p, x);
          const Scalar sw = xaug[inDim];
          for(unsigned k=0;k<y.dim();++k) y[k]*=sw;
          return y;
        };
        LevenbergMarquardtFitter<Scalar> lm(fw, m_outDim);
        beta = lm.fit(xs, ys, beta).params;

        if(m_kernel.type == RobustKernel::None) break;

        // reweight from the current residuals (robust MAD scale)
        std::vector<double> r(N);
        for(int i=0;i<N;++i) r[i] = residual(beta, data[i]);
        const double sigma = RobustKernel::madScale(r);
        Scalar wmax = 0;
        for(int i=0;i<N;++i){ w[i] = Scalar(m_kernel.weight(r[i], sigma)); wmax = std::max(wmax,w[i]); }
        if(wmax <= Scalar(0)) break;   // degenerate scale — stop
      }
      return beta;
    }

    private:
    Function    m_f;
    int         m_inDim, m_outDim;
    Vector      m_start;
    RobustKernel m_kernel;
    int         m_irlsSteps;
  };

} // namespace icl::math
