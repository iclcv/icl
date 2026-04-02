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
      float acc_dot_f(const float* a, const float* b, int n)           { float r; vDSP_dotpr(a,1,b,1,&r,n); return r; }
      float acc_nrm2_f(const float* x, int n)                          { return cblas_snrm2(n,x,1); }
      float acc_asum_f(const float* x, int n)                          { return cblas_sasum(n,x,1); }
      void acc_axpy_f(float a, const float* x, float* y, int n)        { cblas_saxpy(n,a,x,1,y,1); }
      void acc_scal_f(float a, float* x, int n)                        { cblas_sscal(n,a,x,1); }

      // double
      void acc_vadd_d(const double* a, const double* b, double* d, int n) { vDSP_vaddD(a,1,b,1,d,1,n); }
      void acc_vsub_d(const double* a, const double* b, double* d, int n) { vDSP_vsubD(b,1,a,1,d,1,n); }
      void acc_vmul_d(const double* a, const double* b, double* d, int n) { vDSP_vmulD(a,1,b,1,d,1,n); }
      void acc_vdiv_d(const double* a, const double* b, double* d, int n) { vDSP_vdivD(b,1,a,1,d,1,n); }
      void acc_vsadd_d(const double* s, double v, double* d, int n)       { vDSP_vsaddD(s,1,&v,d,1,n); }
      void acc_vsmul_d(const double* s, double v, double* d, int n)       { vDSP_vsmulD(s,1,&v,d,1,n); }
      double acc_dot_d(const double* a, const double* b, int n)           { double r; vDSP_dotprD(a,1,b,1,&r,n); return r; }
      double acc_nrm2_d(const double* x, int n)                          { return cblas_dnrm2(n,x,1); }
      double acc_asum_d(const double* x, int n)                          { return cblas_dasum(n,x,1); }
      void acc_axpy_d(double a, const double* x, double* y, int n)        { cblas_daxpy(n,a,x,1,y,1); }
      void acc_scal_d(double a, double* x, int n)                        { cblas_dscal(n,a,x,1); }

      // ---- Level 2: matrix-vector ----

      void acc_gemv_f(bool trans, int M, int N, float alpha,
                      const float* A, int lda, const float* x, float beta, float* y) {
        cblas_sgemv(CblasRowMajor, trans ? CblasTrans : CblasNoTrans, M, N, alpha, A, lda, x, 1, beta, y, 1);
      }

      void acc_gemv_d(bool trans, int M, int N, double alpha,
                      const double* A, int lda, const double* x, double beta, double* y) {
        cblas_dgemv(CblasRowMajor, trans ? CblasTrans : CblasNoTrans, M, N, alpha, A, lda, x, 1, beta, y, 1);
      }

    } // anonymous namespace

    template<class T>
    void register_all_acc();

    template<>
    void register_all_acc<float>() {
      auto p = BlasOps<float>::instance().backends(Backend::Accelerate);
      // Level 3
      p.add<BlasOps<float>::GemmSig>(BlasOp::gemm, acc_gemm_f, "Accelerate cblas_sgemm");
      // Level 2
      p.add<BlasOps<float>::GemvSig>(BlasOp::gemv, acc_gemv_f, "Accelerate cblas_sgemv");
      // Level 1 binary/scalar
      p.add<BlasOps<float>::VecBinarySig>(BlasOp::vadd, acc_vadd_f, "Accelerate vDSP_vadd");
      p.add<BlasOps<float>::VecBinarySig>(BlasOp::vsub, acc_vsub_f, "Accelerate vDSP_vsub");
      p.add<BlasOps<float>::VecBinarySig>(BlasOp::vmul, acc_vmul_f, "Accelerate vDSP_vmul");
      p.add<BlasOps<float>::VecBinarySig>(BlasOp::vdiv, acc_vdiv_f, "Accelerate vDSP_vdiv");
      p.add<BlasOps<float>::VecScalarSig>(BlasOp::vsadd, acc_vsadd_f, "Accelerate vDSP_vsadd");
      p.add<BlasOps<float>::VecScalarSig>(BlasOp::vsmul, acc_vsmul_f, "Accelerate vDSP_vsmul");
      // Level 1 reductions/in-place
      p.add<BlasOps<float>::DotSig>(BlasOp::dot, acc_dot_f, "Accelerate vDSP_dotpr");
      p.add<BlasOps<float>::NrmSig>(BlasOp::nrm2, acc_nrm2_f, "Accelerate cblas_snrm2");
      p.add<BlasOps<float>::NrmSig>(BlasOp::asum, acc_asum_f, "Accelerate cblas_sasum");
      p.add<BlasOps<float>::AxpySig>(BlasOp::axpy, acc_axpy_f, "Accelerate cblas_saxpy");
      p.add<BlasOps<float>::ScalSig>(BlasOp::scal, acc_scal_f, "Accelerate cblas_sscal");
    }

    template<>
    void register_all_acc<double>() {
      auto p = BlasOps<double>::instance().backends(Backend::Accelerate);
      p.add<BlasOps<double>::GemmSig>(BlasOp::gemm, acc_gemm_d, "Accelerate cblas_dgemm");
      p.add<BlasOps<double>::GemvSig>(BlasOp::gemv, acc_gemv_d, "Accelerate cblas_dgemv");
      p.add<BlasOps<double>::VecBinarySig>(BlasOp::vadd, acc_vadd_d, "Accelerate vDSP_vaddD");
      p.add<BlasOps<double>::VecBinarySig>(BlasOp::vsub, acc_vsub_d, "Accelerate vDSP_vsubD");
      p.add<BlasOps<double>::VecBinarySig>(BlasOp::vmul, acc_vmul_d, "Accelerate vDSP_vmulD");
      p.add<BlasOps<double>::VecBinarySig>(BlasOp::vdiv, acc_vdiv_d, "Accelerate vDSP_vdivD");
      p.add<BlasOps<double>::VecScalarSig>(BlasOp::vsadd, acc_vsadd_d, "Accelerate vDSP_vsaddD");
      p.add<BlasOps<double>::VecScalarSig>(BlasOp::vsmul, acc_vsmul_d, "Accelerate vDSP_vsmulD");
      p.add<BlasOps<double>::DotSig>(BlasOp::dot, acc_dot_d, "Accelerate vDSP_dotprD");
      p.add<BlasOps<double>::NrmSig>(BlasOp::nrm2, acc_nrm2_d, "Accelerate cblas_dnrm2");
      p.add<BlasOps<double>::NrmSig>(BlasOp::asum, acc_asum_d, "Accelerate cblas_dasum");
      p.add<BlasOps<double>::AxpySig>(BlasOp::axpy, acc_axpy_d, "Accelerate cblas_daxpy");
      p.add<BlasOps<double>::ScalSig>(BlasOp::scal, acc_scal_d, "Accelerate cblas_dscal");
    }

    static const int _acc_blas_reg = []() {
      register_all_acc<float>();
      register_all_acc<double>();
      return 0;
    }();

  } // namespace math
} // namespace icl
