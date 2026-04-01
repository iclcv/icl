// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLFilter/AffineOp.h>

namespace icl{
  namespace filter{

    /// Class to scale images \ingroup UNARY \ingroup AFFINE
    class ICLFilter_API ScaleOp : public AffineOp{
      public:
      /// Constructor
      ScaleOp (double factorX=0.0, double factorY=0.0,
               core::scalemode eInterpolate=core::interpolateLIN) :
      AffineOp (eInterpolate) {
        setScale(factorX,factorY);
      }

      /// performs a scale
      /**
        @param factorX scale-factor in x-direction
        @param factorY scale-factor in y-direction
        different values for x and y will lead to a dilation / upsetting deformation
      */
      void setScale (double factorX, double factorY) {
        AffineOp::reset ();
        AffineOp::scale (factorX,factorY);
      }

      // apply should still be public
      ///applies the scale
      using AffineOp::apply;

      private: // hide the following methods
      using AffineOp::rotate;
      using AffineOp::translate;
    };
  } // namespace filter
}
