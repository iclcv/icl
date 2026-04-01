// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/BackendDispatching.h>
#include <ICLUtils/CompatMacros.h>

namespace icl {
  namespace math {

    /// Selector keys for BLAS backend dispatch.
    enum class BlasOp : int {
      gemm,   ///< Level 3: general matrix multiply
      vadd,   ///< Level 1: dst[i] = a[i] + b[i]
      vsub,   ///< Level 1: dst[i] = a[i] - b[i]
      vmul,   ///< Level 1: dst[i] = a[i] * b[i]
      vdiv,   ///< Level 1: dst[i] = a[i] / b[i]
      vsadd,  ///< Level 1: dst[i] = src[i] + scalar
      vsmul,  ///< Level 1: dst[i] = src[i] * scalar
    };

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

      /// Level 3: C = alpha * op(A) * op(B) + beta * C
      using GemmSig = void(bool transA, bool transB,
                            int M, int N, int K, T alpha,
                            const T* A, int lda, const T* B, int ldb,
                            T beta, T* C, int ldc);

      /// Level 1 binary: dst = a OP b (element-wise, n elements)
      using VecBinarySig = void(const T* a, const T* b, T* dst, int n);

      /// Level 1 scalar: dst[i] = src[i] OP scalar (n elements)
      using VecScalarSig = void(const T* src, T scalar, T* dst, int n);

      BlasOps();
      static BlasOps& instance();

      /// Cached dispatch — resolved on first call, O(1) thereafter.
      /// Safe because the context is type-only (no runtime applicability).
      static void vadd(const T* a, const T* b, T* dst, int n);
      static void vsub(const T* a, const T* b, T* dst, int n);
      static void vmul(const T* a, const T* b, T* dst, int n);
      static void vdiv(const T* a, const T* b, T* dst, int n);
      static void vsadd(const T* src, T scalar, T* dst, int n);
      static void vsmul(const T* src, T scalar, T* dst, int n);
    };

  } // namespace math
} // namespace icl
