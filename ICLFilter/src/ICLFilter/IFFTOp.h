/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/IFFTOp.h                       **
** Module : ICLFilter                                              **
** Authors: Christian Groszewski, Christof Elbrechter              **
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

#include <ICLFilter/BaseFFTOp.h>

namespace icl {
  namespace filter {

    /// Inverse 2D FFT/DFT operator
    /** Computes the inverse 2D Fourier transform. See BaseFFTOp for shared
        documentation on result modes and size adaptation.

        Extra features over FFTOp:
        - join: combines pairs of input channels into complex values before IFFT
        - PAD_REMOVE: crops result to a specified ROI (undoes padding from FFTOp)
    */
    class ICLFilter_API IFFTOp : public BaseFFTOp {
    public:
      using BaseFFTOp::ResultMode;
      using BaseFFTOp::SizeAdaptionMode;

      /// Creates a new IFFTOp
      /** @param rm result mode (default: REAL_ONLY)
          @param sam size adaptation mode (default: NO_SCALE)
          @param roi ROI for PAD_REMOVE mode
          @param join whether to join channel pairs into complex (default: true)
          @param ifftshift whether to apply ifftshift before computation (default: true)
          @param forceIDFT whether to force IDFT instead of IFFT (default: false) */
      IFFTOp(ResultMode rm = REAL_ONLY, SizeAdaptionMode sam = NO_SCALE,
             utils::Rect roi = utils::Rect(0, 0, 0, 0), bool join = true,
             bool ifftshift = true, bool forceIDFT = false)
        : BaseFFTOp(true, rm, sam, ifftshift, forceIDFT) {
        setJoinInput(join);
        setRemovePadROI(roi);
      }

      /// Backward-compatible alias
      void setForceIDFT(bool f) { setForceDFT(f); }
      /// Backward-compatible alias
      bool getForceIDFT() { return getForceDFT(); }
      /// Backward-compatible alias
      void setJoinMatrix(bool j) { setJoinInput(j); }
      /// Backward-compatible alias
      bool getJoinMatrix() { return getJoinInput(); }
      /// Backward-compatible alias
      void setROI(utils::Rect r) { setRemovePadROI(r); }
      /// Backward-compatible alias
      utils::Rect getRoi() { return getRemovePadROI(); }
    };

  } // namespace filter
}
