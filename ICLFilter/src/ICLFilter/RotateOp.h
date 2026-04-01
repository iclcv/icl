// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLFilter/AffineOp.h>

namespace icl{
  namespace filter{

   /// Class to rotate images \ingroup UNARY \ingroup AFFINE
    class ICLFilter_API RotateOp : public AffineOp {
      public:
      /// Constructor
      RotateOp (double dAngle=0.0, core::scalemode eInterpolate=core::interpolateLIN) :
        AffineOp (eInterpolate) {
          setAngle(dAngle);
        }

      /// sets the rotation angle
      /**
        @param dAngle angle in degrees (clockwise)
      */
      void setAngle (double dAngle) {
        AffineOp::reset ();
        AffineOp::rotate (dAngle);
      }

      // apply should still be public
      ///applies the rotation
      using AffineOp::apply;

      private: // hide the following methods
      using AffineOp::translate;
      using AffineOp::scale;
    };
  } // namespace filter
}
