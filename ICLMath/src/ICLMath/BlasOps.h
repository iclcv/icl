// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/BackendDispatching.h>
#include <ICLUtils/CompatMacros.h>

namespace icl {
  namespace math {

    /// Selector keys for BLAS backend dispatch (Level 1, 2, 3).
    enum class BlasOp : int {
      // Level 3 — matrix-matrix, O(n³)
      gemm,   ///< C = α·op(A)·op(B) + β·C

      // Level 2 — matrix-vector, O(n²)
      gemv,   ///< y = α·A·x + β·y

      // Level 1 — vector-vector, O(n)
      vadd,   ///< dst[i] = a[i] + b[i]
      vsub,   ///< dst[i] = a[i] - b[i]
      vmul,   ///< dst[i] = a[i] * b[i]
      vdiv,   ///< dst[i] = a[i] / b[i]
      vsadd,  ///< dst[i] = src[i] + scalar
      vsmul,  ///< dst[i] = src[i] * scalar
      dot,    ///< return Σ(a[i]·b[i])
      nrm2,   ///< return √Σ(x[i]²)
      asum,   ///< return Σ|x[i]|
      axpy,   ///< y[i] += α·x[i]
      scal,   ///< x[i] *= α
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

      // ---- Level 3 signatures ----
      using GemmSig = void(bool transA, bool transB,
                            int M, int N, int K, T alpha,
                            const T* A, int lda, const T* B, int ldb,
                            T beta, T* C, int ldc);

      // ---- Level 2 signatures ----
      using GemvSig = void(bool trans, int M, int N, T alpha,
                            const T* A, int lda, const T* x,
                            T beta, T* y);

      // ---- Level 1 signatures ----
      using VecBinarySig = void(const T* a, const T* b, T* dst, int n);
      using VecScalarSig = void(const T* src, T scalar, T* dst, int n);
      using DotSig  = T(const T* a, const T* b, int n);
      using NrmSig  = T(const T* x, int n);
      using AxpySig = void(T alpha, const T* x, T* y, int n);
      using ScalSig = void(T alpha, T* x, int n);

      BlasOps();
      static BlasOps& instance();

      // ---- Cached dispatch — resolved on first call, O(1) thereafter ----

      // Level 1 element-wise
      static void vadd(const T* a, const T* b, T* dst, int n);
      static void vsub(const T* a, const T* b, T* dst, int n);
      static void vmul(const T* a, const T* b, T* dst, int n);
      static void vdiv(const T* a, const T* b, T* dst, int n);
      static void vsadd(const T* src, T scalar, T* dst, int n);
      static void vsmul(const T* src, T scalar, T* dst, int n);

      // Level 1 reductions & in-place
      static T    dot(const T* a, const T* b, int n);
      static T    nrm2(const T* x, int n);
      static T    asum(const T* x, int n);
      static void axpy(T alpha, const T* x, T* y, int n);
      static void scal(T alpha, T* x, int n);

      // Level 2
      static void gemv(bool trans, int M, int N, T alpha,
                        const T* A, int lda, const T* x,
                        T beta, T* y);
    };

  } // namespace math
} // namespace icl
