// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLFilter/BaseAffineOp.h>
#include <ICLCore/Image.h>
#include <ICLCore/ImageBackendDispatching.h>

namespace icl{
  namespace filter{
    /// Class to apply an arbitrary series of affine transformations \ingroup AFFINE \ingroup UNARY
    /** @see AffineOp.h for full documentation */
    class ICLFilter_API AffineOp : public BaseAffineOp, public core::ImageBackendDispatching {
      public:
      AffineOp(const AffineOp&) = delete;
      AffineOp& operator=(const AffineOp&) = delete;

      /// Backend selector keys
      enum class Op : int { apply };

      /// Dispatch signature: src, dst, forward 2x3 matrix (6 doubles row-major), interpolation
      using AffineSig = void(const core::Image&, core::Image&, const double*, core::scalemode);

      /// Class-level prototype — owns selectors, populated during static init
      static core::ImageBackendDispatching& prototype();

      /// Constructor
      AffineOp (core::scalemode eInterpolate=core::interpolateLIN);
      /// resets the internal Matrix
      void reset  ();
      /// adds a rotation (angle in degrees, clockwise)
      void rotate (double dAngle);

      /// adds a translation
      void translate (double x, double y) {
        m_aadT[0][2] += x; m_aadT[1][2] += y;
      }
      /// adds a scale
      void scale (double x, double y) {
        m_aadT[0][0] *= x; m_aadT[1][0] *= x;
        m_aadT[0][1] *= y; m_aadT[1][1] *= y;
      }

      /// Applies the affine transform to the image
      void apply(const core::Image &src, core::Image &dst) override;

      /// import from super-class
      using BaseAffineOp::apply;

      /// sets whether the result image is scaled and translated to contain the whole result image
      inline void setAdaptResultImage(bool on){ m_adaptResultImage = on; }
      /// returns the Adapt Result image option
      inline bool getAdaptResultDisplay() const{ return m_adaptResultImage; }

      private:
      void applyT (const double p[2], double aResult[2]);
      static void useMinMax (const double aCur[2],
                             double aMin[2], double aMax[2]);
      void getShiftAndSize (const utils::Rect& roi, utils::Size& size,
                            double& xShift, double& yShift);
      double    m_aadT[2][3];
      core::scalemode m_eInterpolate;
      bool m_adaptResultImage;
    };

    /// ADL-visible toString for AffineOp::Op (defined in AffineOp.cpp)
    ICLFilter_API const char* toString(AffineOp::Op op);

  } // namespace filter
}
