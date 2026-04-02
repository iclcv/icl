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
      // Inline so the compiler can fold the static-local check + virtual call
      // into the caller without an extra function-call layer.

#define BLAS_CACHED_INLINE_BIN(NAME, KEY) \
      static inline void NAME(const T* a, const T* b, T* dst, int n) { \
        static auto* impl = instance().template getSelector<VecBinarySig>(BlasOp::KEY).resolveOrThrow(); \
        impl->apply(a, b, dst, n); \
      }
#define BLAS_CACHED_INLINE_SCAL(NAME, KEY) \
      static inline void NAME(const T* src, T scalar, T* dst, int n) { \
        static auto* impl = instance().template getSelector<VecScalarSig>(BlasOp::KEY).resolveOrThrow(); \
        impl->apply(src, scalar, dst, n); \
      }

      // Level 1 element-wise
      BLAS_CACHED_INLINE_BIN(vadd, vadd)
      BLAS_CACHED_INLINE_BIN(vsub, vsub)
      BLAS_CACHED_INLINE_BIN(vmul, vmul)
      BLAS_CACHED_INLINE_BIN(vdiv, vdiv)
      BLAS_CACHED_INLINE_SCAL(vsadd, vsadd)
      BLAS_CACHED_INLINE_SCAL(vsmul, vsmul)

#undef BLAS_CACHED_INLINE_BIN
#undef BLAS_CACHED_INLINE_SCAL

      // Level 1 reductions & in-place
      static inline T dot(const T* a, const T* b, int n) {
        static auto* impl = instance().template getSelector<DotSig>(BlasOp::dot).resolveOrThrow();
        return impl->apply(a, b, n);
      }
      static inline T nrm2(const T* x, int n) {
        static auto* impl = instance().template getSelector<NrmSig>(BlasOp::nrm2).resolveOrThrow();
        return impl->apply(x, n);
      }
      static inline T asum(const T* x, int n) {
        static auto* impl = instance().template getSelector<NrmSig>(BlasOp::asum).resolveOrThrow();
        return impl->apply(x, n);
      }
      static inline void axpy(T alpha, const T* x, T* y, int n) {
        static auto* impl = instance().template getSelector<AxpySig>(BlasOp::axpy).resolveOrThrow();
        impl->apply(alpha, x, y, n);
      }
      static inline void scal(T alpha, T* x, int n) {
        static auto* impl = instance().template getSelector<ScalSig>(BlasOp::scal).resolveOrThrow();
        impl->apply(alpha, x, n);
      }

      // Level 2
      static inline void gemv(bool trans, int M, int N, T alpha,
                               const T* A, int lda, const T* x,
                               T beta, T* y) {
        static auto* impl = instance().template getSelector<GemvSig>(BlasOp::gemv).resolveOrThrow();
        impl->apply(trans, M, N, alpha, A, lda, x, beta, y);
      }
    };

  } // namespace math
} // namespace icl
