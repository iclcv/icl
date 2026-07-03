// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/math/fit/ModelFitter.h>
#include <icl/math/fit/LeastSquareModelFitting2D.h>
#include <icl/math/fit/FitUtils.h>
#include <algorithm>
#include <cmath>

namespace icl::math {

  /// ModelFitter face over the algebraic 2D primitive fits (line/circle/ellipse).
  /** The Model is the implicit-equation coefficient vector produced by
      LeastSquareModelFitting2D (e.g. circle: a(x²+y²)+bx+cy+d=0); residual() is the
      algebraic distance. These are the concrete Tier-A fitters that RobustFitter
      wraps to get outlier-tolerant primitive fitting. */
  class PrimitiveFitter2D : public ModelFitter<utils::Point32f, std::vector<double> > {
    protected:
    LeastSquareModelFitting2D m_ls;
    int m_modelDim;
    public:
    using Model = std::vector<double>;
    PrimitiveFitter2D(int modelDim, LeastSquareModelFitting2D::DesignMatrixGen gen)
      : m_ls(modelDim, gen), m_modelDim(modelDim){}

    Model fit(const std::vector<utils::Point32f> &pts) override {
      return m_ls.fit(pts);
    }
    double residual(const Model &m, const utils::Point32f &p) const override {
      return m_ls.getError(m, p);
    }
    /// minimal points = model dimension - 1 (homogeneous coefficient vector)
    int minSamples() const override { return m_modelDim - 1; }
  };

  /// algebraic straight-line fit  (model: [a,b,c] for a·x + b·y + c = 0)
  struct LineFitter2D : public PrimitiveFitter2D {
    LineFitter2D() : PrimitiveFitter2D(3, LeastSquareModelFitting2D::line_gen){}
  };

  /// algebraic circle fit  (model: [a,b,c,d] for a(x²+y²) + b·x + c·y + d = 0)
  struct CircleFitter2D : public PrimitiveFitter2D {
    CircleFitter2D() : PrimitiveFitter2D(4, LeastSquareModelFitting2D::circle_gen){}
  };

  /// algebraic general-ellipse fit  (model: [a..f] for a·x²+b·xy+c·y²+d·x+e·y+f=0)
  struct EllipseFitter2D : public PrimitiveFitter2D {
    EllipseFitter2D() : PrimitiveFitter2D(6, LeastSquareModelFitting2D::ellipse_gen){}
  };

  /// Taubin algebraic circle fit — a strictly better circle fitter.
  /** Near-geometric accuracy at algebraic cost and far less biased than the naive
      (Kåsa-style) LeastSquareModelFitting circle fit for partial arcs / noisy data
      (Taubin 1991; Chernov). Internally centers + scales the data and solves the
      normalized null-space (via homogeneousNullSpace). The Model is the same
      [a,b,c,d] coefficient vector as CircleFitter2D (a≡1), so it is a drop-in
      replacement anywhere — including as a RobustFitter base or SeededFitter seed. */
  class TaubinCircleFitter : public ModelFitter<utils::Point32f, std::vector<double> > {
    public:
    using Model = std::vector<double>;

    Model fit(const std::vector<utils::Point32f> &pts) override {
      const int N = int(pts.size());
      double mx = 0, my = 0;
      for(const auto &p : pts){ mx += p.x; my += p.y; }
      mx /= N; my /= N;

      std::vector<double> X(N), Y(N), Z(N);
      double Zmean = 0;
      for(int i = 0; i < N; ++i){
        X[i] = pts[i].x - mx; Y[i] = pts[i].y - my;
        Z[i] = X[i]*X[i] + Y[i]*Y[i]; Zmean += Z[i];
      }
      Zmean /= N;
      const double sZ = std::sqrt(Zmean);

      // 3x3 scatter of the normalized design rows [ (Z-Zmean)/(2√Zmean), X, Y ]
      DynMatrix<double> M = DynMatrix<double>::create(3, 3, 0.0);
      for(int i = 0; i < N; ++i){
        const double row[3] = { (Z[i]-Zmean)/(2*sZ), X[i], Y[i] };
        for(int p = 0; p < 3; ++p) for(int q = 0; q < 3; ++q) M(p,q) += row[p]*row[q];
      }
      const DynColVector<double> A = homogeneousNullSpace(M);
      const double a = A[0]/(2*sZ), A1 = A[1], A2 = A[2], dc = -Zmean*a;
      const double Xc = -A1/(2*a), Yc = -A2/(2*a);
      const double r  = std::sqrt(std::max(0.0, A1*A1 + A2*A2 - 4*a*dc)) / (2*std::abs(a));
      const double cx = Xc + mx, cy = Yc + my;
      return { 1.0, -2*cx, -2*cy, cx*cx + cy*cy - r*r };
    }
    double residual(const Model &m, const utils::Point32f &p) const override {
      return std::abs(m[0]*(p.x*p.x + p.y*p.y) + m[1]*p.x + m[2]*p.y + m[3]);
    }
    int minSamples() const override { return 3; }
  };

  /// Halíř–Flusser direct ellipse fit — numerically stable, guarantees an ellipse.
  /** The stable form (Halíř & Flusser 1998) of Fitzgibbon's constrained fit: it
      splits the scatter matrix to avoid inverting a singular block and imposes the
      4ac−b² = 1 constraint, so the result is ALWAYS an ellipse — unlike
      EllipseFitter2D's identity-constraint fit, which can return a hyperbola on
      noisy / partial data. Needs a non-symmetric eigensolver (DynMatrix::eigenGeneral
      / LAPACK geev). Model is the same [A,B,C,D,E,F] coefficient vector as
      EllipseFitter2D (A·x²+B·xy+C·y²+D·x+E·y+F=0), so it drops in anywhere. */
  class HalirFlusserEllipseFitter : public ModelFitter<utils::Point32f, std::vector<double> > {
    public:
    using Model = std::vector<double>;

    Model fit(const std::vector<utils::Point32f> &pts) override {
      const int N = int(pts.size());
      DynMatrix<double> D1(N,3), D2(N,3);            // (rows,cols)
      for(int i=0;i<N;++i){
        const double x=pts[i].x, y=pts[i].y;
        D1(i,0)=x*x; D1(i,1)=x*y; D1(i,2)=y*y;
        D2(i,0)=x;   D2(i,1)=y;   D2(i,2)=1.0;
      }
      const DynMatrix<double> S1 = D1.transp()*D1;   // 3×3 quadratic scatter
      const DynMatrix<double> S2 = D1.transp()*D2;   // 3×3
      const DynMatrix<double> S3 = D2.transp()*D2;   // 3×3
      const DynMatrix<double> T  = (S3.inv()*S2.transp()) * -1.0;   // T = -S3⁻¹ S2ᵀ
      const DynMatrix<double> M  = S1 + S2*T;
      // reduced system C1⁻¹·M with the ellipse constraint C1 = [[0,0,2],[0,-1,0],[2,0,0]]
      DynMatrix<double> C1inv = DynMatrix<double>::create(3,3,0.0);
      C1inv(0,2)=0.5; C1inv(1,1)=-1.0; C1inv(2,0)=0.5;
      const DynMatrix<double> M2 = C1inv*M;

      const auto e = M2.eigenGeneral();
      Model a1(3, 0.0);                               // pick the eigenvector with 4ac−b²>0
      for(int j=0;j<3;++j){
        if(std::abs(e.valuesImag[j]) > 1e-9) continue;
        const double v0=e.vectorsReal(0,j), v1=e.vectorsReal(1,j), v2=e.vectorsReal(2,j);
        if(4.0*v0*v2 - v1*v1 > 0){ a1 = {v0,v1,v2}; break; }
      }
      DynMatrix<double> a1v(3,1); for(int i=0;i<3;++i) a1v[i]=a1[i];
      const DynMatrix<double> a2 = T*a1v;             // [D,E,F] = T·[A,B,C]
      return { a1[0], a1[1], a1[2], a2[0], a2[1], a2[2] };
    }
    double residual(const Model &m, const utils::Point32f &p) const override {
      const double x=p.x, y=p.y;
      return std::abs(m[0]*x*x + m[1]*x*y + m[2]*y*y + m[3]*x + m[4]*y + m[5]);
    }
    int minSamples() const override { return 5; }
  };

} // namespace icl::math
