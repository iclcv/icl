// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/BackendDispatching.h>
#include <ICLUtils/CompatMacros.h>
#include <complex>

namespace icl::math {
  /// Selector keys for FFT backend dispatch.
  enum class FFTOp : int { r2c, c2c, inv_c2c };

  ICLMath_API const char* toString(FFTOp op);

  /// FFT dispatch — parameterized on scalar type (float or double).
  /// Operates on raw data pointers. DynMatrix wrapping stays in FFTUtils.
  ///
  /// All operations work on row-major 2D data of size rows x cols.
  /// Output must be pre-allocated by the caller (rows * cols complex values).
  ///
  /// Backends: C++ fallback (always), MKL DFTI, FFTW, Accelerate vDSP.
  /// Context is int (unused — no applicability checks needed).
  template<class T>
  struct ICLMath_API FFTOps : utils::BackendDispatching<int> {
    using C = std::complex<T>;

    /// Real-to-complex 2D forward FFT.
    /// src: row-major T[rows*cols], dst: pre-allocated C[rows*cols]
    using R2CSig = void(const T* src, int rows, int cols, C* dst);

    /// Complex-to-complex 2D forward FFT.
    /// src: row-major C[rows*cols], dst: pre-allocated C[rows*cols]
    using C2CSig = void(const C* src, int rows, int cols, C* dst);

    /// Complex-to-complex 2D inverse FFT (includes 1/(rows*cols) normalization).
    /// src: row-major C[rows*cols], dst: pre-allocated C[rows*cols]
    using InvC2CSig = void(const C* src, int rows, int cols, C* dst);

    FFTOps();
    static FFTOps& instance();
  };

  } // namespace icl::math