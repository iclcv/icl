/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/FFTOp.h                        **
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

#include <ICLFilter/BaseFFTOp.h>

namespace icl {
  namespace filter {

    /// Forward 2D FFT/DFT operator
    /** Computes the 2D Fourier transform using IPP, MKL, or a pure C++ fallback.
        See BaseFFTOp for result modes and size adaptation modes.

        If the data size is a power of 2, the fast Fourier transform (FFT) is used.
        Otherwise, the discrete Fourier transform (DFT) is used unless forceDFT is set.

        If IPP is available and the size is power-of-2, IPP acceleration is used.
        If MKL is available, it handles arbitrary sizes. Otherwise the C++ fallback
        is used (significantly slower for large images).
    */
    class ICLFilter_API FFTOp : public BaseFFTOp {
    public:
      using BaseFFTOp::ResultMode;
      using BaseFFTOp::SizeAdaptionMode;

      /// Creates a new FFTOp
      /** @param rm result mode (default: LOG_POWER_SPECTRUM)
          @param sam size adaptation mode (default: NO_SCALE)
          @param fftshift whether to apply fftshift to output (default: true)
          @param forceDFT whether to force DFT instead of FFT (default: false) */
      FFTOp(ResultMode rm = LOG_POWER_SPECTRUM, SizeAdaptionMode sam = NO_SCALE,
            bool fftshift = true, bool forceDFT = false)
        : BaseFFTOp(false, rm, sam, fftshift, forceDFT) {}

      /// Backward-compatible alias
      void setFFTShift(bool s) { setShift(s); }
      /// Backward-compatible alias
      bool getFFTShift() { return getShift(); }
    };

  } // namespace filter
}
