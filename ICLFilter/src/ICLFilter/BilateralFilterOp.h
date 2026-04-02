// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Tobias Roehlig, Christof Elbrechter

#pragma once

#include <ICLFilter/UnaryOp.h>
#include <ICLCore/Image.h>
#include <ICLCore/ImageBackendDispatching.h>

namespace icl::filter {
  /// Gaussian bilateral filter — edge-preserving smoothing
  /** The bilateral filter combines spatial Gaussian weighting with
      range (intensity/color difference) weighting. Pixels that are
      both spatially close and similar in value contribute more to
      the output, preserving edges while smoothing flat regions.

      For 3-channel images, setting use_lab=true converts to CIE LAB
      color space for the range distance computation, which gives
      more perceptually uniform edge preservation.

      Supported depths: all (8u, 16s, 32s, 32f, 64f) via C++ backend.
      OpenCL backend accelerates 8u (mono + 3ch) and 32f (mono).
  */
  class ICLFilter_API BilateralFilterOp : public UnaryOp, public core::ImageBackendDispatching {
  public:

    /// Backend selector keys
    enum class Op : int { apply };

    /// Dispatch signature for backend implementations
    using ApplySig = void(const core::Image&, core::Image&,
                          int radius, float sigma_s, float sigma_r, bool use_lab);

    /// Constructor with all parameters
    BilateralFilterOp(int radius = 2, float sigma_s = 2.f, float sigma_r = 30.f,
                      bool use_lab = true);

    /// Class-level prototype — owns selectors, populated during static init
    static core::ImageBackendDispatching& prototype();

    using UnaryOp::apply;
    void apply(const core::Image &src, core::Image &dst) override;

    void setRadius(int r) { m_radius = r; }
    void setSigmaS(float s) { m_sigmaS = s; }
    void setSigmaR(float r) { m_sigmaR = r; }
    void setUseLAB(bool b) { m_useLAB = b; }

    int getRadius() const { return m_radius; }
    float getSigmaS() const { return m_sigmaS; }
    float getSigmaR() const { return m_sigmaR; }
    bool getUseLAB() const { return m_useLAB; }

  private:
    int m_radius;
    float m_sigmaS, m_sigmaR;
    bool m_useLAB;
  };

  /// ADL-visible toString for BilateralFilterOp::Op → registry name (defined in BilateralFilterOp.cpp)
  ICLFilter_API const char* toString(BilateralFilterOp::Op op);

} // namespace icl::filter