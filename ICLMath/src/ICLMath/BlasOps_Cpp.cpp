// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// C++ fallback backends for BLAS/LAPACK operations.
// Contains naive GEMM and Golub-Kahan bidiagonalization SVD.

#include <ICLMath/BlasOps.h>
#include <ICLMath/DynMatrix.h>

#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>

using namespace icl::utils;

namespace icl::math {
  namespace {

    // ================================================================
    // GEMM: naive O(M*N*K) — correct but slow
    // ================================================================


    enum class AlphaBeta{
      Zero,
      One,
      Other
    };

    template<class T, bool transA, bool transB, AlphaBeta withAlpha, AlphaBeta withBeta>
    void cpp_gemm_ta_tb_wa_wb(int M, int N, int K, T alpha,
                  const T* A, int lda, const T* B, int ldb,
                  T beta, T* C, int ldc) {
      for(int i = 0; i < M; ++i) {
        for(int j = 0; j < N; ++j) {
          T sum = 0;
          for(int k = 0; k < K; ++k) {
            T a = transA ? A[k * lda + i] : A[i * lda + k];
            T b = transB ? B[j * ldb + k] : B[k * ldb + j];
            sum += a * b;
          }
          T aterm;
          if constexpr (withAlpha == AlphaBeta::Zero) aterm = 0;
          else if constexpr (withAlpha == AlphaBeta::One) aterm = sum;
          else aterm = alpha * sum;

          if constexpr (withBeta == AlphaBeta::Zero) C[i * ldc + j] = aterm;
          else if constexpr (withBeta == AlphaBeta::One) C[i * ldc + j] = aterm + C[i * ldc + j];
          else C[i * ldc + j] = aterm + beta * C[i * ldc + j];
        }
      }
    }

    template<class T>
    bool isZero(T v) { return std::abs(v) < std::numeric_limits<T>::epsilon(); }

    template<class T>
    bool isOne(T v) { return std::abs(v - T(1)) < std::numeric_limits<T>::epsilon(); }

    template<class T, bool transA, bool transB>
    void cpp_gemm_ta_tb(int M, int N, int K, T alpha,
                       const T* A, int lda, const T* B, int ldb,
                       T beta, T* C, int ldc) {
#define CALL_GEMM(wa, wb) cpp_gemm_ta_tb_wa_wb<T, transA, transB, AlphaBeta::wa, AlphaBeta::wb>(M, N, K, alpha, A, lda, B, ldb, beta, C, ldc)
      if (isZero(alpha)){
        if (isZero(beta)) CALL_GEMM(Zero, Zero);
        else if (isOne(beta)) CALL_GEMM(Zero, One);
        else CALL_GEMM(Zero, Other);
      } else if (isOne(alpha)) {
        if (isZero(beta)) CALL_GEMM(One, Zero);
        else if (isOne(beta)) CALL_GEMM(One, One);
        else CALL_GEMM(One, Other);
      } else {
        if (isZero(beta)) CALL_GEMM(Other, Zero);
        else if (isOne(beta)) CALL_GEMM(Other, One);
        else CALL_GEMM(Other, Other);
      }
#undef CALL_GEMM
    }

    template<class T>
    void cpp_gemm(bool transA, bool transB,
                  int M, int N, int K, T alpha,
                  const T* A, int lda, const T* B, int ldb,
                  T beta, T* C, int ldc) {
#define CALL_GEMM(ta, tb) cpp_gemm_ta_tb<T, ta, tb>(M, N, K, alpha, A, lda, B, ldb, beta, C, ldc)
      if (transA && transB) CALL_GEMM(true, true);
      else if (transA && !transB) CALL_GEMM(true, false);
      else if (!transA && transB) CALL_GEMM(false, true);
      else CALL_GEMM(false, false);
#undef CALL_GEMM
    }

  } // anonymous namespace

  // ================================================================
  // Level 1: element-wise vector operations
  // ================================================================

  // ================================================================
  // Level 1: element-wise + reductions + in-place
  // ================================================================

  template<class T> void cpp_vadd(const T* a, const T* b, T* d, int n) { for(int i=0;i<n;++i) d[i]=a[i]+b[i]; }
  template<class T> void cpp_vsub(const T* a, const T* b, T* d, int n) { for(int i=0;i<n;++i) d[i]=a[i]-b[i]; }
  template<class T> void cpp_vmul(const T* a, const T* b, T* d, int n) { for(int i=0;i<n;++i) d[i]=a[i]*b[i]; }
  template<class T> void cpp_vdiv(const T* a, const T* b, T* d, int n) { for(int i=0;i<n;++i) d[i]=a[i]/b[i]; }
  template<class T> void cpp_vsadd(const T* s, T v, T* d, int n) { for(int i=0;i<n;++i) d[i]=s[i]+v; }
  template<class T> void cpp_vsmul(const T* s, T v, T* d, int n) { for(int i=0;i<n;++i) d[i]=s[i]*v; }

  template<class T> T cpp_dot(const T* a, const T* b, int n) {
    T s = 0; for(int i=0;i<n;++i) s += a[i]*b[i]; return s;
  }
  template<class T> T cpp_nrm2(const T* x, int n) {
    T s = 0; for(int i=0;i<n;++i) s += x[i]*x[i]; return std::sqrt(s);
  }
  template<class T> T cpp_asum(const T* x, int n) {
    T s = 0; for(int i=0;i<n;++i) s += std::abs(x[i]); return s;
  }
  template<class T> void cpp_axpy(T alpha, const T* x, T* y, int n) {
    for(int i=0;i<n;++i) y[i] += alpha * x[i];
  }
  template<class T> void cpp_scal(T alpha, T* x, int n) {
    for(int i=0;i<n;++i) x[i] *= alpha;
  }

  // ================================================================
  // Level 2: matrix-vector
  // ================================================================

  template<class T>
  void cpp_gemv(bool trans, int M, int N, T alpha,
                const T* A, int lda, const T* x, T beta, T* y) {
    int yLen = trans ? N : M;
    for(int i = 0; i < yLen; ++i) y[i] *= beta;
    if(trans) {
      // y = alpha * A^T * x + beta*y  (y is Nx1, x is Mx1)
      for(int j = 0; j < N; ++j)
        for(int i = 0; i < M; ++i)
          y[j] += alpha * A[i * lda + j] * x[i];
    } else {
      // y = alpha * A * x + beta*y  (y is Mx1, x is Nx1)
      for(int i = 0; i < M; ++i)
        for(int j = 0; j < N; ++j)
          y[i] += alpha * A[i * lda + j] * x[j];
    }
  }

  // ================================================================
  // Registration
  // ================================================================

  template<class T>
  void register_all_cpp() {
    auto p = BlasOps<T>::instance().backends(Backend::Cpp);
    // Level 3
    p.template add<typename BlasOps<T>::GemmSig>(BlasOp::gemm, cpp_gemm<T>, "C++ GEMM");
    // Level 2
    p.template add<typename BlasOps<T>::GemvSig>(BlasOp::gemv, cpp_gemv<T>, "C++ GEMV");
    // Level 1 binary/scalar
    p.template add<typename BlasOps<T>::VecBinarySig>(BlasOp::vadd, cpp_vadd<T>, "C++ vadd");
    p.template add<typename BlasOps<T>::VecBinarySig>(BlasOp::vsub, cpp_vsub<T>, "C++ vsub");
    p.template add<typename BlasOps<T>::VecBinarySig>(BlasOp::vmul, cpp_vmul<T>, "C++ vmul");
    p.template add<typename BlasOps<T>::VecBinarySig>(BlasOp::vdiv, cpp_vdiv<T>, "C++ vdiv");
    p.template add<typename BlasOps<T>::VecScalarSig>(BlasOp::vsadd, cpp_vsadd<T>, "C++ vsadd");
    p.template add<typename BlasOps<T>::VecScalarSig>(BlasOp::vsmul, cpp_vsmul<T>, "C++ vsmul");
    // Level 1 reductions/in-place
    p.template add<typename BlasOps<T>::DotSig>(BlasOp::dot, cpp_dot<T>, "C++ dot");
    p.template add<typename BlasOps<T>::NrmSig>(BlasOp::nrm2, cpp_nrm2<T>, "C++ nrm2");
    p.template add<typename BlasOps<T>::NrmSig>(BlasOp::asum, cpp_asum<T>, "C++ asum");
    p.template add<typename BlasOps<T>::AxpySig>(BlasOp::axpy, cpp_axpy<T>, "C++ axpy");
    p.template add<typename BlasOps<T>::ScalSig>(BlasOp::scal, cpp_scal<T>, "C++ scal");
  }

  static const int _cpp_blas_reg = []() {
    register_all_cpp<float>();
    register_all_cpp<double>();
    return 0;
  }();

  } // namespace icl::math