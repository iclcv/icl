// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

// Apple Accelerate backend for BLAS/LAPACK operations.
// This file is excluded from the build when Accelerate is not found.

#include <ICLMath/BlasOps.h>
#include <Accelerate/Accelerate.h>
#include <vector>
#include <algorithm>

using namespace icl::utils;

namespace icl {
  namespace math {

    namespace {

      // ---- GEMM (identical CBLAS interface) ----

      void acc_gemm_f(bool transA, bool transB,
                      int M, int N, int K, float alpha,
                      const float* A, int lda, const float* B, int ldb,
                      float beta, float* C, int ldc) {
        cblas_sgemm(CblasRowMajor,
                    transA ? CblasTrans : CblasNoTrans,
                    transB ? CblasTrans : CblasNoTrans,
                    M, N, K, alpha, A, lda, B, ldb, beta, C, ldc);
      }

      void acc_gemm_d(bool transA, bool transB,
                      int M, int N, int K, double alpha,
                      const double* A, int lda, const double* B, int ldb,
                      double beta, double* C, int ldc) {
        cblas_dgemm(CblasRowMajor,
                    transA ? CblasTrans : CblasNoTrans,
                    transB ? CblasTrans : CblasNoTrans,
                    M, N, K, alpha, A, lda, B, ldb, beta, C, ldc);
      }

    } // anonymous namespace

    static const int _acc_blas_reg = []() {
      auto acc_f = BlasOps<float>::instance().backends(Backend::Accelerate);
      acc_f.add<BlasOps<float>::GemmSig>(BlasOp::gemm, acc_gemm_f, "Accelerate cblas_sgemm");

      auto acc_d = BlasOps<double>::instance().backends(Backend::Accelerate);
      acc_d.add<BlasOps<double>::GemmSig>(BlasOp::gemm, acc_gemm_d, "Accelerate cblas_dgemm");

      return 0;
    }();

  } // namespace math
} // namespace icl
