// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/BackendDispatching.h>
#include <ICLUtils/CompatMacros.h>

namespace icl {
  namespace math {

    /// Selector keys for BLAS backend dispatch.
    enum class BlasOp : int { gemm };

    ICLMath_API const char* toString(BlasOp op);

    /// BLAS dispatch — parameterized on scalar type (float or double).
    /// Operates on raw data pointers. Higher-level DynMatrix wrapping stays
    /// in consumer code (DynMatrix.cpp, DynMatrixUtils.cpp).
    ///
    /// Backends: C++ fallback (always), MKL, Accelerate, OpenBLAS.
    /// Context is int (unused — no applicability checks needed).
    ///
    /// Note: LAPACK operations (gesdd, syev, etc.) are in LapackOps.
    template<class T>
    struct ICLMath_API BlasOps : utils::BackendDispatching<int> {

      /// General matrix multiply: C = alpha * op(A) * op(B) + beta * C
      /// transA/transB: false = no transpose, true = transpose
      /// M = rows of op(A), N = cols of op(B), K = cols of op(A) = rows of op(B)
      using GemmSig = void(bool transA, bool transB,
                            int M, int N, int K, T alpha,
                            const T* A, int lda, const T* B, int ldb,
                            T beta, T* C, int ldc);

      BlasOps();
      static BlasOps& instance();
    };

  } // namespace math
} // namespace icl
