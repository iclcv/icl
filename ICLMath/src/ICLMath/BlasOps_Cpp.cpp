/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/BlasOps_Cpp.cpp                    **
** Module : ICLMath                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

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
