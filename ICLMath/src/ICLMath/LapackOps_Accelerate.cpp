/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/LapackOps_Accelerate.cpp           **
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

// Apple Accelerate backend for LAPACK operations.
// This file is excluded from the build when Accelerate is not found.

#include <ICLMath/LapackOps.h>
#include <Accelerate/Accelerate.h>
#include <vector>
#include <algorithm>

using namespace icl::utils;

namespace icl {
  namespace math {

    namespace {

      // ---- GESDD (SVD) ----
      // Row-major ICL convention: swap M<->N and U<->Vt.

      int acc_gesdd_f(char jobz, int M, int N, float* A, int lda,
                      float* S, float* U, int ldu, float* Vt, int ldvt) {
        __LAPACK_int info;
        __LAPACK_int n = N, m = M, _lda = lda, _ldu = ldu, _ldvt = ldvt;
        __LAPACK_int mn = std::min(M, N);
        std::vector<__LAPACK_int> iwork(std::max(1, 8 * (int)mn));

        float work_query;
        __LAPACK_int lwork = -1;
        sgesdd_(&jobz, &n, &m, A, &_lda, S, Vt, &_ldvt, U, &_ldu,
                &work_query, &lwork, iwork.data(), &info);
        if(info != 0) return info;

        lwork = static_cast<__LAPACK_int>(work_query);
        std::vector<float> work(lwork);

        sgesdd_(&jobz, &n, &m, A, &_lda, S, Vt, &_ldvt, U, &_ldu,
                work.data(), &lwork, iwork.data(), &info);
        return info;
      }

      int acc_gesdd_d(char jobz, int M, int N, double* A, int lda,
                      double* S, double* U, int ldu, double* Vt, int ldvt) {
        __LAPACK_int info;
        __LAPACK_int n = N, m = M, _lda = lda, _ldu = ldu, _ldvt = ldvt;
        __LAPACK_int mn = std::min(M, N);
        std::vector<__LAPACK_int> iwork(std::max(1, 8 * (int)mn));

        double work_query;
        __LAPACK_int lwork = -1;
        dgesdd_(&jobz, &n, &m, A, &_lda, S, Vt, &_ldvt, U, &_ldu,
                &work_query, &lwork, iwork.data(), &info);
        if(info != 0) return info;

        lwork = static_cast<__LAPACK_int>(work_query);
        std::vector<double> work(lwork);

        dgesdd_(&jobz, &n, &m, A, &_lda, S, Vt, &_ldvt, U, &_ldu,
                work.data(), &lwork, iwork.data(), &info);
        return info;
      }

      // ---- SYEV (symmetric eigenvalue decomposition) ----

      int acc_syev_f(char jobz, int N, float* A, int lda, float* W) {
        __LAPACK_int info;
        __LAPACK_int n = N, _lda = lda;
        char uplo = 'U';

        float work_query;
        __LAPACK_int lwork = -1;
        ssyev_(&jobz, &uplo, &n, A, &_lda, W, &work_query, &lwork, &info);
        if(info != 0) return info;

        lwork = static_cast<__LAPACK_int>(work_query);
        std::vector<float> work(lwork);

        ssyev_(&jobz, &uplo, &n, A, &_lda, W, work.data(), &lwork, &info);
        return info;
      }

      int acc_syev_d(char jobz, int N, double* A, int lda, double* W) {
        __LAPACK_int info;
        __LAPACK_int n = N, _lda = lda;
        char uplo = 'U';

        double work_query;
        __LAPACK_int lwork = -1;
        dsyev_(&jobz, &uplo, &n, A, &_lda, W, &work_query, &lwork, &info);
        if(info != 0) return info;

        lwork = static_cast<__LAPACK_int>(work_query);
        std::vector<double> work(lwork);

        dsyev_(&jobz, &uplo, &n, A, &_lda, W, work.data(), &lwork, &info);
        return info;
      }

      // ---- GETRF (LU factorization) ----

      int acc_getrf_f(int M, int N, float* A, int lda, int* ipiv) {
        __LAPACK_int info, m = M, n = N, _lda = lda;
        std::vector<__LAPACK_int> lipiv(std::min(M, N));
        sgetrf_(&n, &m, A, &_lda, lipiv.data(), &info);
        for(int i = 0; i < std::min(M, N); i++) ipiv[i] = lipiv[i];
        return info;
      }

      int acc_getrf_d(int M, int N, double* A, int lda, int* ipiv) {
        __LAPACK_int info, m = M, n = N, _lda = lda;
        std::vector<__LAPACK_int> lipiv(std::min(M, N));
        dgetrf_(&n, &m, A, &_lda, lipiv.data(), &info);
        for(int i = 0; i < std::min(M, N); i++) ipiv[i] = lipiv[i];
        return info;
      }

      // ---- GETRI (inverse from LU) ----

      int acc_getri_f(int N, float* A, int lda, const int* ipiv) {
        __LAPACK_int info, n = N, _lda = lda;
        std::vector<__LAPACK_int> lipiv(N);
        for(int i = 0; i < N; i++) lipiv[i] = ipiv[i];

        float work_query;
        __LAPACK_int lwork = -1;
        sgetri_(&n, A, &_lda, lipiv.data(), &work_query, &lwork, &info);
        if(info != 0) return info;

        lwork = static_cast<__LAPACK_int>(work_query);
        std::vector<float> work(lwork);
        sgetri_(&n, A, &_lda, lipiv.data(), work.data(), &lwork, &info);
        return info;
      }

      int acc_getri_d(int N, double* A, int lda, const int* ipiv) {
        __LAPACK_int info, n = N, _lda = lda;
        std::vector<__LAPACK_int> lipiv(N);
        for(int i = 0; i < N; i++) lipiv[i] = ipiv[i];

        double work_query;
        __LAPACK_int lwork = -1;
        dgetri_(&n, A, &_lda, lipiv.data(), &work_query, &lwork, &info);
        if(info != 0) return info;

        lwork = static_cast<__LAPACK_int>(work_query);
        std::vector<double> work(lwork);
        dgetri_(&n, A, &_lda, lipiv.data(), work.data(), &lwork, &info);
        return info;
      }

    } // anonymous namespace

    static const int _acc_lapack_reg = []() {
      auto acc_f = LapackOps<float>::instance().backends(Backend::Accelerate);
      acc_f.add<LapackOps<float>::GesddSig>(LapackOp::gesdd, acc_gesdd_f, "Accelerate sgesdd");
      acc_f.add<LapackOps<float>::SyevSig>(LapackOp::syev, acc_syev_f, "Accelerate ssyev");
      acc_f.add<LapackOps<float>::GetrfSig>(LapackOp::getrf, acc_getrf_f, "Accelerate sgetrf");
      acc_f.add<LapackOps<float>::GetriSig>(LapackOp::getri, acc_getri_f, "Accelerate sgetri");

      auto acc_d = LapackOps<double>::instance().backends(Backend::Accelerate);
      acc_d.add<LapackOps<double>::GesddSig>(LapackOp::gesdd, acc_gesdd_d, "Accelerate dgesdd");
      acc_d.add<LapackOps<double>::SyevSig>(LapackOp::syev, acc_syev_d, "Accelerate dsyev");
      acc_d.add<LapackOps<double>::GetrfSig>(LapackOp::getrf, acc_getrf_d, "Accelerate dgetrf");
      acc_d.add<LapackOps<double>::GetriSig>(LapackOp::getri, acc_getri_d, "Accelerate dgetri");

      return 0;
    }();

  } // namespace math
} // namespace icl
