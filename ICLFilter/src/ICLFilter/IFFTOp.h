// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christian Groszewski, Christof Elbrechter

#pragma once

#include <ICLFilter/BaseFFTOp.h>

namespace icl::filter {
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

  } // namespace icl::filter