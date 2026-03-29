/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/AffineOp.h                     **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter                                    **
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

      /// Dispatch signature: src, dst, forward 2x3 matrix (6 doubles row-major), interpolation
      using AffineSig = void(const core::Image&, core::Image&, const double*, core::scalemode);

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
  } // namespace filter
}
