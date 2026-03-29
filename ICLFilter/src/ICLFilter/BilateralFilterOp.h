/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/BilateralFilterOp.h            **
** Module : ICLFilter                                              **
** Authors: Tobias Roehlig, Christof Elbrechter                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <ICLFilter/UnaryOp.h>
#include <ICLCore/Image.h>
#include <ICLCore/ImageBackendDispatching.h>

namespace icl {
namespace filter {

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

    /// Dispatch signature for backend implementations
    using ApplySig = void(const core::Image&, core::Image&,
                          int radius, float sigma_s, float sigma_r, bool use_lab);

    /// Constructor with all parameters
    BilateralFilterOp(int radius = 2, float sigma_s = 2.f, float sigma_r = 30.f,
                      bool use_lab = true);

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

} // namespace filter
} // namespace icl
