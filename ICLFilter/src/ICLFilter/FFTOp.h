// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLFilter/BaseFFTOp.h>

namespace icl::filter {
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

  } // namespace icl::filter