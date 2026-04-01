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

      // ================================================================
      // Row-major ↔ column-major:
      // LAPACK expects column-major; our raw pointer interface is row-major.
      // For gesdd/syev the transposition is handled via dimension/argument
      // swaps. For getrf/getri/geqrf/orgqr we transpose explicitly so that
      // the packed output (L/U, Householder reflectors) matches the C++
      // backend convention.
      // ================================================================

      // ---- GETRF (LU factorization) ----

      int acc_getrf_f(int M, int N, float* A, int lda, int* ipiv) {
        int mn = std::min(M, N);
        std::vector<float> AT(M * N);
        for(int i = 0; i < M; i++)
          for(int j = 0; j < N; j++)
            AT[j * M + i] = A[i * lda + j];
        __LAPACK_int info, m = M, n = N, _lda = M;
        std::vector<__LAPACK_int> lipiv(mn);
        sgetrf_(&m, &n, AT.data(), &_lda, lipiv.data(), &info);
        for(int i = 0; i < M; i++)
          for(int j = 0; j < N; j++)
            A[i * lda + j] = AT[j * M + i];
        for(int i = 0; i < mn; i++) ipiv[i] = lipiv[i];
        return info;
      }

      int acc_getrf_d(int M, int N, double* A, int lda, int* ipiv) {
        int mn = std::min(M, N);
        std::vector<double> AT(M * N);
        for(int i = 0; i < M; i++)
          for(int j = 0; j < N; j++)
            AT[j * M + i] = A[i * lda + j];
        __LAPACK_int info, m = M, n = N, _lda = M;
        std::vector<__LAPACK_int> lipiv(mn);
        dgetrf_(&m, &n, AT.data(), &_lda, lipiv.data(), &info);
        for(int i = 0; i < M; i++)
          for(int j = 0; j < N; j++)
            A[i * lda + j] = AT[j * M + i];
        for(int i = 0; i < mn; i++) ipiv[i] = lipiv[i];
        return info;
      }

      // ---- GETRI (inverse from LU) ----

      int acc_getri_f(int N, float* A, int lda, const int* ipiv) {
        std::vector<float> AT(N * N);
        for(int i = 0; i < N; i++)
          for(int j = 0; j < N; j++)
            AT[j * N + i] = A[i * lda + j];
        __LAPACK_int info, n = N, _lda = N;
        std::vector<__LAPACK_int> lipiv(N);
        for(int i = 0; i < N; i++) lipiv[i] = ipiv[i];
        float work_query;
        __LAPACK_int lwork = -1;
        sgetri_(&n, AT.data(), &_lda, lipiv.data(), &work_query, &lwork, &info);
        if(info != 0) return info;
        lwork = static_cast<__LAPACK_int>(work_query);
        std::vector<float> work(lwork);
        sgetri_(&n, AT.data(), &_lda, lipiv.data(), work.data(), &lwork, &info);
        if(info != 0) return info;
        for(int i = 0; i < N; i++)
          for(int j = 0; j < N; j++)
            A[i * lda + j] = AT[j * N + i];
        return 0;
      }

      int acc_getri_d(int N, double* A, int lda, const int* ipiv) {
        std::vector<double> AT(N * N);
        for(int i = 0; i < N; i++)
          for(int j = 0; j < N; j++)
            AT[j * N + i] = A[i * lda + j];
        __LAPACK_int info, n = N, _lda = N;
        std::vector<__LAPACK_int> lipiv(N);
        for(int i = 0; i < N; i++) lipiv[i] = ipiv[i];
        double work_query;
        __LAPACK_int lwork = -1;
        dgetri_(&n, AT.data(), &_lda, lipiv.data(), &work_query, &lwork, &info);
        if(info != 0) return info;
        lwork = static_cast<__LAPACK_int>(work_query);
        std::vector<double> work(lwork);
        dgetri_(&n, AT.data(), &_lda, lipiv.data(), work.data(), &lwork, &info);
        if(info != 0) return info;
        for(int i = 0; i < N; i++)
          for(int j = 0; j < N; j++)
            A[i * lda + j] = AT[j * N + i];
        return 0;
      }

      // ---- GEQRF (QR factorization) ----

      int acc_geqrf_f(int M, int N, float* A, int lda, float* tau) {
        std::vector<float> AT(M * N);
        for(int i = 0; i < M; i++)
          for(int j = 0; j < N; j++)
            AT[j * M + i] = A[i * lda + j];
        __LAPACK_int info, m = M, n = N, _lda = M;
        float work_query;
        __LAPACK_int lwork = -1;
        sgeqrf_(&m, &n, AT.data(), &_lda, tau, &work_query, &lwork, &info);
        if(info != 0) return info;
        lwork = static_cast<__LAPACK_int>(work_query);
        std::vector<float> work(lwork);
        sgeqrf_(&m, &n, AT.data(), &_lda, tau, work.data(), &lwork, &info);
        if(info != 0) return info;
        for(int i = 0; i < M; i++)
          for(int j = 0; j < N; j++)
            A[i * lda + j] = AT[j * M + i];
        return 0;
      }

      int acc_geqrf_d(int M, int N, double* A, int lda, double* tau) {
        std::vector<double> AT(M * N);
        for(int i = 0; i < M; i++)
          for(int j = 0; j < N; j++)
            AT[j * M + i] = A[i * lda + j];
        __LAPACK_int info, m = M, n = N, _lda = M;
        double work_query;
        __LAPACK_int lwork = -1;
        dgeqrf_(&m, &n, AT.data(), &_lda, tau, &work_query, &lwork, &info);
        if(info != 0) return info;
        lwork = static_cast<__LAPACK_int>(work_query);
        std::vector<double> work(lwork);
        dgeqrf_(&m, &n, AT.data(), &_lda, tau, work.data(), &lwork, &info);
        if(info != 0) return info;
        for(int i = 0; i < M; i++)
          for(int j = 0; j < N; j++)
            A[i * lda + j] = AT[j * M + i];
        return 0;
      }

      // ---- ORGQR (form Q from Householder reflectors) ----

      int acc_orgqr_f(int M, int N, int K, float* A, int lda, const float* tau) {
        std::vector<float> AT(M * N);
        for(int i = 0; i < M; i++)
          for(int j = 0; j < N; j++)
            AT[j * M + i] = A[i * lda + j];
        __LAPACK_int info, m = M, n = N, k = K, _lda = M;
        std::vector<float> tau_copy(tau, tau + K);
        float work_query;
        __LAPACK_int lwork = -1;
        sorgqr_(&m, &n, &k, AT.data(), &_lda, tau_copy.data(), &work_query, &lwork, &info);
        if(info != 0) return info;
        lwork = static_cast<__LAPACK_int>(work_query);
        std::vector<float> work(lwork);
        sorgqr_(&m, &n, &k, AT.data(), &_lda, tau_copy.data(), work.data(), &lwork, &info);
        if(info != 0) return info;
        for(int i = 0; i < M; i++)
          for(int j = 0; j < N; j++)
            A[i * lda + j] = AT[j * M + i];
        return 0;
      }

      int acc_orgqr_d(int M, int N, int K, double* A, int lda, const double* tau) {
        std::vector<double> AT(M * N);
        for(int i = 0; i < M; i++)
          for(int j = 0; j < N; j++)
            AT[j * M + i] = A[i * lda + j];
        __LAPACK_int info, m = M, n = N, k = K, _lda = M;
        std::vector<double> tau_copy(tau, tau + K);
        double work_query;
        __LAPACK_int lwork = -1;
        dorgqr_(&m, &n, &k, AT.data(), &_lda, tau_copy.data(), &work_query, &lwork, &info);
        if(info != 0) return info;
        lwork = static_cast<__LAPACK_int>(work_query);
        std::vector<double> work(lwork);
        dorgqr_(&m, &n, &k, AT.data(), &_lda, tau_copy.data(), work.data(), &lwork, &info);
        if(info != 0) return info;
        for(int i = 0; i < M; i++)
          for(int j = 0; j < N; j++)
            A[i * lda + j] = AT[j * M + i];
        return 0;
      }

      // ---- GELSD (least-squares solve via SVD) ----

      int acc_gelsd_f(int M, int N, int NRHS, float* A, int lda,
                      float* B, int ldb, float* S, float rcond, int* rank) {
        int mx = std::max(M, N);
        std::vector<float> AT(M * N);
        for(int i = 0; i < M; i++)
          for(int j = 0; j < N; j++)
            AT[j * M + i] = A[i * lda + j];
        // B is max(M,N)×NRHS row-major → transpose to column-major max(M,N)×NRHS
        std::vector<float> BT(mx * NRHS);
        for(int i = 0; i < mx; i++)
          for(int j = 0; j < NRHS; j++)
            BT[j * mx + i] = B[i * ldb + j];
        __LAPACK_int info, m = M, n = N, nrhs = NRHS, _lda = M, _ldb = mx;
        __LAPACK_int lrank;
        float work_query;
        __LAPACK_int lwork = -1;
        __LAPACK_int iwork_query;
        sgelsd_(&m, &n, &nrhs, AT.data(), &_lda, BT.data(), &_ldb,
                S, &rcond, &lrank, &work_query, &lwork, &iwork_query, &info);
        if(info != 0) return info;
        lwork = static_cast<__LAPACK_int>(work_query);
        std::vector<float> work(lwork);
        std::vector<__LAPACK_int> iwork(iwork_query);
        sgelsd_(&m, &n, &nrhs, AT.data(), &_lda, BT.data(), &_ldb,
                S, &rcond, &lrank, work.data(), &lwork, iwork.data(), &info);
        *rank = lrank;
        // Transpose solution back: first N rows of BT (column-major) → B (row-major)
        for(int i = 0; i < N; i++)
          for(int j = 0; j < NRHS; j++)
            B[i * ldb + j] = BT[j * mx + i];
        return info;
      }

      int acc_gelsd_d(int M, int N, int NRHS, double* A, int lda,
                      double* B, int ldb, double* S, double rcond, int* rank) {
        int mx = std::max(M, N);
        std::vector<double> AT(M * N);
        for(int i = 0; i < M; i++)
          for(int j = 0; j < N; j++)
            AT[j * M + i] = A[i * lda + j];
        std::vector<double> BT(mx * NRHS);
        for(int i = 0; i < mx; i++)
          for(int j = 0; j < NRHS; j++)
            BT[j * mx + i] = B[i * ldb + j];
        __LAPACK_int info, m = M, n = N, nrhs = NRHS, _lda = M, _ldb = mx;
        __LAPACK_int lrank;
        double work_query;
        __LAPACK_int lwork = -1;
        __LAPACK_int iwork_query;
        dgelsd_(&m, &n, &nrhs, AT.data(), &_lda, BT.data(), &_ldb,
                S, &rcond, &lrank, &work_query, &lwork, &iwork_query, &info);
        if(info != 0) return info;
        lwork = static_cast<__LAPACK_int>(work_query);
        std::vector<double> work(lwork);
        std::vector<__LAPACK_int> iwork(iwork_query);
        dgelsd_(&m, &n, &nrhs, AT.data(), &_lda, BT.data(), &_ldb,
                S, &rcond, &lrank, work.data(), &lwork, iwork.data(), &info);
        *rank = lrank;
        for(int i = 0; i < N; i++)
          for(int j = 0; j < NRHS; j++)
            B[i * ldb + j] = BT[j * mx + i];
        return info;
      }

    } // anonymous namespace

    static const int _acc_lapack_reg = []() {
      auto acc_f = LapackOps<float>::instance().backends(Backend::Accelerate);
      acc_f.add<LapackOps<float>::GesddSig>(LapackOp::gesdd, acc_gesdd_f, "Accelerate sgesdd");
      acc_f.add<LapackOps<float>::SyevSig>(LapackOp::syev, acc_syev_f, "Accelerate ssyev");
      acc_f.add<LapackOps<float>::GetrfSig>(LapackOp::getrf, acc_getrf_f, "Accelerate sgetrf");
      acc_f.add<LapackOps<float>::GetriSig>(LapackOp::getri, acc_getri_f, "Accelerate sgetri");
      acc_f.add<LapackOps<float>::GeqrfSig>(LapackOp::geqrf, acc_geqrf_f, "Accelerate sgeqrf");
      acc_f.add<LapackOps<float>::OrgqrSig>(LapackOp::orgqr, acc_orgqr_f, "Accelerate sorgqr");
      acc_f.add<LapackOps<float>::GelsdSig>(LapackOp::gelsd, acc_gelsd_f, "Accelerate sgelsd");

      auto acc_d = LapackOps<double>::instance().backends(Backend::Accelerate);
      acc_d.add<LapackOps<double>::GesddSig>(LapackOp::gesdd, acc_gesdd_d, "Accelerate dgesdd");
      acc_d.add<LapackOps<double>::SyevSig>(LapackOp::syev, acc_syev_d, "Accelerate dsyev");
      acc_d.add<LapackOps<double>::GetrfSig>(LapackOp::getrf, acc_getrf_d, "Accelerate dgetrf");
      acc_d.add<LapackOps<double>::GetriSig>(LapackOp::getri, acc_getri_d, "Accelerate dgetri");
      acc_d.add<LapackOps<double>::GeqrfSig>(LapackOp::geqrf, acc_geqrf_d, "Accelerate dgeqrf");
      acc_d.add<LapackOps<double>::OrgqrSig>(LapackOp::orgqr, acc_orgqr_d, "Accelerate dorgqr");
      acc_d.add<LapackOps<double>::GelsdSig>(LapackOp::gelsd, acc_gelsd_d, "Accelerate dgelsd");

      return 0;
    }();

  } // namespace math
} // namespace icl
