// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christian Groszewski, Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLMath/FFTUtils.h>
#include <ICLCore/Img.h>
#include <ICLFilter/UnaryOp.h>
#include <ICLCore/Image.h>

namespace icl::filter {
    /// Common base class for FFTOp and IFFTOp
    /** Holds unified enums, DynMatrix buffers, size adaptation, result mode
        extraction, and the shift implementation. Subclasses only set
        `inverse` in the constructor and provide backward-compatible API. */
    class ICLFilter_API BaseFFTOp : public UnaryOp {
    public:

      /// How the result image is constructed from the complex FFT output
      enum ResultMode {
        TWO_CHANNEL_COMPLEX,         ///< real and imaginary as separate channels
        IMAG_ONLY,                   ///< imaginary part only
        REAL_ONLY,                   ///< real part only
        POWER_SPECTRUM,              ///< |z|^2
        LOG_POWER_SPECTRUM,          ///< log(1 + |z|^2)
        MAGNITUDE_ONLY,              ///< |z|
        PHASE_ONLY,                  ///< atan2(imag, real)
        TWO_CHANNEL_MAGNITUDE_PHASE  ///< magnitude and phase as separate channels
      };

      /// How the source image is adapted before FFT computation
      enum SizeAdaptionMode {
        NO_SCALE,    ///< keep original size
        PAD_ZERO,    ///< zero-pad to next power of 2 (forward only)
        PAD_COPY,    ///< tile-copy to next power of 2 (forward only)
        PAD_MIRROR,  ///< mirror-pad to next power of 2 (forward only)
        SCALE_UP,    ///< interpolate to next power of 2
        SCALE_DOWN,  ///< downsample to prior power of 2
        PAD_REMOVE   ///< remove padding via ROI (inverse only)
      };

      void setResultMode(ResultMode rm);
      ResultMode getResultMode() const;

      void setSizeAdaptionMode(SizeAdaptionMode sam);
      SizeAdaptionMode getSizeAdaptionMode() const;

      void setForceDFT(bool f);
      bool getForceDFT() const;

      void setShift(bool s);
      bool getShift() const;

      bool isInverse() const;

      /// Set whether to join two input channels into one complex (inverse only)
      void setJoinInput(bool join);
      bool getJoinInput() const;

      /// Set ROI for PAD_REMOVE mode (inverse only)
      void setRemovePadROI(const utils::Rect& roi);
      utils::Rect getRemovePadROI() const;

      void apply(const core::Image& src, core::Image& dst) override;
      using UnaryOp::apply;

    protected:
      BaseFFTOp(bool inverse, ResultMode rm, SizeAdaptionMode sam,
                bool shift, bool forceDFT);
      ~BaseFFTOp();

    private:
      struct Data;
      Data* m_data;

      template<class T>
      const core::Img<T>* adaptSource(const core::Img<T>* src);

      template<class SrcT, class DstT>
      void applyInternal(const core::Img<SrcT>& src, core::Img<DstT>& dst,
                         math::DynMatrix<std::complex<DstT>>& buf,
                         math::DynMatrix<std::complex<DstT>>& dstBuf);

      template<class T>
      void applyFftShift(math::DynMatrix<T>& m);

      template<class T>
      void applyIfftShift(math::DynMatrix<T>& m);
    };

  } // namespace icl::filter