// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLFilter/AffineOp.h>

namespace icl::filter {
  /// Class to translate images \ingroup UNARY \ingroup AFFINE
  /** TODO: currently the translation effect is compensated by the AffineOp's
      re-centering mechanism*/
  class ICLFilter_API TranslateOp : public AffineOp {
    public:
    /// Constructor
    TranslateOp (double dX=0.0, double dY=0.0, core::scalemode eInterpolate=core::interpolateLIN) :
      AffineOp (eInterpolate) {
        setTranslation(dX,dY);
        setAdaptResultImage(false);
      }

    /// performs a translation
    /**
      @param dX pixels to translate in x-direction
      @param dY pixels to translate in y-direction
    */

    void setTranslation (double dX, double dY) {
      AffineOp::reset ();
      AffineOp::translate (dX,dY);
    }

    // apply should still be public

    ///applies the translation
    using AffineOp::apply;

    private: // hide the following methods
    using AffineOp::rotate;
    using AffineOp::scale;
    using AffineOp::setAdaptResultImage;
  };
  } // namespace icl::filter