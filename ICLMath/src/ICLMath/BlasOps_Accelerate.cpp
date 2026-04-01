// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
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

      // ---- Level 1: element-wise vector ops via vDSP ----

      // float
      void acc_vadd_f(const float* a, const float* b, float* d, int n) { vDSP_vadd(a,1,b,1,d,1,n); }
      void acc_vsub_f(const float* a, const float* b, float* d, int n) { vDSP_vsub(b,1,a,1,d,1,n); }
      void acc_vmul_f(const float* a, const float* b, float* d, int n) { vDSP_vmul(a,1,b,1,d,1,n); }
      void acc_vdiv_f(const float* a, const float* b, float* d, int n) { vDSP_vdiv(b,1,a,1,d,1,n); }
      void acc_vsadd_f(const float* s, float v, float* d, int n)       { vDSP_vsadd(s,1,&v,d,1,n); }
      void acc_vsmul_f(const float* s, float v, float* d, int n)       { vDSP_vsmul(s,1,&v,d,1,n); }

      // double
      void acc_vadd_d(const double* a, const double* b, double* d, int n) { vDSP_vaddD(a,1,b,1,d,1,n); }
      void acc_vsub_d(const double* a, const double* b, double* d, int n) { vDSP_vsubD(b,1,a,1,d,1,n); }
      void acc_vmul_d(const double* a, const double* b, double* d, int n) { vDSP_vmulD(a,1,b,1,d,1,n); }
      void acc_vdiv_d(const double* a, const double* b, double* d, int n) { vDSP_vdivD(b,1,a,1,d,1,n); }
      void acc_vsadd_d(const double* s, double v, double* d, int n)       { vDSP_vsaddD(s,1,&v,d,1,n); }
      void acc_vsmul_d(const double* s, double v, double* d, int n)       { vDSP_vsmulD(s,1,&v,d,1,n); }

    } // anonymous namespace

    static const int _acc_blas_reg = []() {
      auto acc_f = BlasOps<float>::instance().backends(Backend::Accelerate);
      acc_f.add<BlasOps<float>::GemmSig>(BlasOp::gemm, acc_gemm_f, "Accelerate cblas_sgemm");
      acc_f.add<BlasOps<float>::VecBinarySig>(BlasOp::vadd, acc_vadd_f, "Accelerate vDSP_vadd");
      acc_f.add<BlasOps<float>::VecBinarySig>(BlasOp::vsub, acc_vsub_f, "Accelerate vDSP_vsub");
      acc_f.add<BlasOps<float>::VecBinarySig>(BlasOp::vmul, acc_vmul_f, "Accelerate vDSP_vmul");
      acc_f.add<BlasOps<float>::VecBinarySig>(BlasOp::vdiv, acc_vdiv_f, "Accelerate vDSP_vdiv");
      acc_f.add<BlasOps<float>::VecScalarSig>(BlasOp::vsadd, acc_vsadd_f, "Accelerate vDSP_vsadd");
      acc_f.add<BlasOps<float>::VecScalarSig>(BlasOp::vsmul, acc_vsmul_f, "Accelerate vDSP_vsmul");

      auto acc_d = BlasOps<double>::instance().backends(Backend::Accelerate);
      acc_d.add<BlasOps<double>::GemmSig>(BlasOp::gemm, acc_gemm_d, "Accelerate cblas_dgemm");
      acc_d.add<BlasOps<double>::VecBinarySig>(BlasOp::vadd, acc_vadd_d, "Accelerate vDSP_vaddD");
      acc_d.add<BlasOps<double>::VecBinarySig>(BlasOp::vsub, acc_vsub_d, "Accelerate vDSP_vsubD");
      acc_d.add<BlasOps<double>::VecBinarySig>(BlasOp::vmul, acc_vmul_d, "Accelerate vDSP_vmulD");
      acc_d.add<BlasOps<double>::VecBinarySig>(BlasOp::vdiv, acc_vdiv_d, "Accelerate vDSP_vdivD");
      acc_d.add<BlasOps<double>::VecScalarSig>(BlasOp::vsadd, acc_vsadd_d, "Accelerate vDSP_vsaddD");
      acc_d.add<BlasOps<double>::VecScalarSig>(BlasOp::vsmul, acc_vsmul_d, "Accelerate vDSP_vsmulD");

      return 0;
    }();

  } // namespace math
} // namespace icl
