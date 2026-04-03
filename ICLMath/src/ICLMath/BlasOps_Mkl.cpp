// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// MKL backend for BLAS/LAPACK operations.
// This file is excluded from the build when MKL is not found.

#include <ICLMath/BlasOps.h>

#include "mkl_cblas.h"
#include "mkl_lapack.h"

#include <vector>
#include <algorithm>

using namespace icl::utils;

namespace icl::math {
  namespace {

    // ---- GEMM ----

    void mkl_gemm_f(bool transA, bool transB,
                    int M, int N, int K, float alpha,
                    const float* A, int lda, const float* B, int ldb,
                    float beta, float* C, int ldc) {
      cblas_sgemm(CblasRowMajor,
                  transA ? CblasTrans : CblasNoTrans,
                  transB ? CblasTrans : CblasNoTrans,
                  M, N, K, alpha, A, lda, B, ldb, beta, C, ldc);
    }

    void mkl_gemm_d(bool transA, bool transB,
                    int M, int N, int K, double alpha,
                    const double* A, int lda, const double* B, int ldb,
                    double beta, double* C, int ldc) {
      cblas_dgemm(CblasRowMajor,
                  transA ? CblasTrans : CblasNoTrans,
                  transB ? CblasTrans : CblasNoTrans,
                  M, N, K, alpha, A, lda, B, ldb, beta, C, ldc);
    }

  } // anonymous namespace

  static const int _mkl_blas_reg = []() {
    auto mkl_f = BlasOps<float>::instance().backends(Backend::Mkl);
    mkl_f.add<BlasOps<float>::GemmSig>(BlasOp::gemm, mkl_gemm_f, "MKL cblas_sgemm");

    auto mkl_d = BlasOps<double>::instance().backends(Backend::Mkl);
    mkl_d.add<BlasOps<double>::GemmSig>(BlasOp::gemm, mkl_gemm_d, "MKL cblas_dgemm");

    return 0;
  }();

  } // namespace icl::math