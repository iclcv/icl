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

namespace icl {
  namespace math {

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

    static const int _cpp_blas_reg = []() {
      auto cpp_f = BlasOps<float>::instance().backends(Backend::Cpp);
      cpp_f.add<BlasOps<float>::GemmSig>(BlasOp::gemm, cpp_gemm<float>, "C++ naive GEMM");

      auto cpp_d = BlasOps<double>::instance().backends(Backend::Cpp);
      cpp_d.add<BlasOps<double>::GemmSig>(BlasOp::gemm, cpp_gemm<double>, "C++ naive GEMM");

      return 0;
    }();

  } // namespace math
} // namespace icl
