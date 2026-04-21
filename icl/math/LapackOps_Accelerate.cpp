// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Apple Accelerate backend for LAPACK operations.
// This file is excluded from the build when Accelerate is not found.

#include <icl/math/LapackOps.h>
#include <Accelerate/Accelerate.h>
#include <vector>
#include <algorithm>

using namespace icl::utils;

namespace icl::math {
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
    // swaps. For getrf/getri/geqrf/orgqr/gelsd we transpose explicitly
    // using the lapack_row_to_col / lapack_col_to_row helpers so that
    // the packed output (L/U, Householder reflectors) matches the C++
    // backend convention.
    // ================================================================

    // ---- GETRF (LU factorization) ----

    int acc_getrf_f(int M, int N, float* A, int lda, int* ipiv) {
      int mn = std::min(M, N);
      auto AT = lapack_row_to_col(A, M, N, lda);
      __LAPACK_int info, m = M, n = N, _lda = M;
      std::vector<__LAPACK_int> lipiv(mn);
      sgetrf_(&m, &n, AT.data(), &_lda, lipiv.data(), &info);
      lapack_col_to_row(AT.data(), M, N, A, lda);
      for(int i = 0; i < mn; i++) ipiv[i] = lipiv[i];
      return info;
    }

    int acc_getrf_d(int M, int N, double* A, int lda, int* ipiv) {
      int mn = std::min(M, N);
      auto AT = lapack_row_to_col(A, M, N, lda);
      __LAPACK_int info, m = M, n = N, _lda = M;
      std::vector<__LAPACK_int> lipiv(mn);
      dgetrf_(&m, &n, AT.data(), &_lda, lipiv.data(), &info);
      lapack_col_to_row(AT.data(), M, N, A, lda);
      for(int i = 0; i < mn; i++) ipiv[i] = lipiv[i];
      return info;
    }

    // ---- GETRI (inverse from LU) ----

    int acc_getri_f(int N, float* A, int lda, const int* ipiv) {
      auto AT = lapack_row_to_col(A, N, N, lda);
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
      lapack_col_to_row(AT.data(), N, N, A, lda);
      return 0;
    }

    int acc_getri_d(int N, double* A, int lda, const int* ipiv) {
      auto AT = lapack_row_to_col(A, N, N, lda);
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
      lapack_col_to_row(AT.data(), N, N, A, lda);
      return 0;
    }

    // ---- GEQRF (QR factorization) ----

    int acc_geqrf_f(int M, int N, float* A, int lda, float* tau) {
      auto AT = lapack_row_to_col(A, M, N, lda);
      __LAPACK_int info, m = M, n = N, _lda = M;
      float work_query;
      __LAPACK_int lwork = -1;
      sgeqrf_(&m, &n, AT.data(), &_lda, tau, &work_query, &lwork, &info);
      if(info != 0) return info;
      lwork = static_cast<__LAPACK_int>(work_query);
      std::vector<float> work(lwork);
      sgeqrf_(&m, &n, AT.data(), &_lda, tau, work.data(), &lwork, &info);
      if(info != 0) return info;
      lapack_col_to_row(AT.data(), M, N, A, lda);
      return 0;
    }

    int acc_geqrf_d(int M, int N, double* A, int lda, double* tau) {
      auto AT = lapack_row_to_col(A, M, N, lda);
      __LAPACK_int info, m = M, n = N, _lda = M;
      double work_query;
      __LAPACK_int lwork = -1;
      dgeqrf_(&m, &n, AT.data(), &_lda, tau, &work_query, &lwork, &info);
      if(info != 0) return info;
      lwork = static_cast<__LAPACK_int>(work_query);
      std::vector<double> work(lwork);
      dgeqrf_(&m, &n, AT.data(), &_lda, tau, work.data(), &lwork, &info);
      if(info != 0) return info;
      lapack_col_to_row(AT.data(), M, N, A, lda);
      return 0;
    }

    // ---- ORGQR (form Q from Householder reflectors) ----

    int acc_orgqr_f(int M, int N, int K, float* A, int lda, const float* tau) {
      auto AT = lapack_row_to_col(A, M, N, lda);
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
      lapack_col_to_row(AT.data(), M, N, A, lda);
      return 0;
    }

    int acc_orgqr_d(int M, int N, int K, double* A, int lda, const double* tau) {
      auto AT = lapack_row_to_col(A, M, N, lda);
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
      lapack_col_to_row(AT.data(), M, N, A, lda);
      return 0;
    }

    // ---- GELSD (least-squares solve via SVD) ----

    int acc_gelsd_f(int M, int N, int NRHS, float* A, int lda,
                    float* B, int ldb, float* S, float rcond, int* rank) {
      int mx = std::max(M, N);
      auto AT = lapack_row_to_col(A, M, N, lda);
      auto BT = lapack_row_to_col(B, mx, NRHS, ldb);
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
      // Copy first N rows from column-major BT (stride mx) back to row-major B.
      // Can't use lapack_col_to_row here: BT stride is mx, not N.
      for(int i = 0; i < N; i++)
        for(int j = 0; j < NRHS; j++)
          B[i * ldb + j] = BT[j * mx + i];
      return info;
    }

    int acc_gelsd_d(int M, int N, int NRHS, double* A, int lda,
                    double* B, int ldb, double* S, double rcond, int* rank) {
      int mx = std::max(M, N);
      auto AT = lapack_row_to_col(A, M, N, lda);
      auto BT = lapack_row_to_col(B, mx, NRHS, ldb);
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

  } // namespace icl::math
