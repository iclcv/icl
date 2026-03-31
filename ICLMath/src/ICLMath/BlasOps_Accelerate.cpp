/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/BlasOps_Accelerate.cpp             **
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
