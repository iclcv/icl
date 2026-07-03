// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/math/fit/ModelFitter.h>
#include <icl/math/fit/LeastSquareModelFitting2D.h>

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

} // namespace icl::math
