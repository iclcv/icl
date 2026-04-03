// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// MKL backend for LAPACK operations.
// This file is excluded from the build when MKL is not found.

#include <ICLMath/LapackOps.h>
#include "mkl_lapack.h"
#include <vector>
#include <algorithm>

using namespace icl::utils;

namespace icl::math {
  namespace {

    // ---- GESDD (SVD) ----

    int mkl_gesdd_f(char jobz, int M, int N, float* A, int lda,
                    float* S, float* U, int ldu, float* Vt, int ldvt) {
      int info;
      int mn = std::min(M, N);
      std::vector<int> iwork(std::max(1, 8 * mn));

      float work_query;
      int lwork = -1;
      sgesdd(&jobz, &N, &M, A, &lda, S, Vt, &ldvt, U, &ldu,
             &work_query, &lwork, iwork.data(), &info);
      if(info != 0) return info;

      lwork = static_cast<int>(work_query);
      std::vector<float> work(lwork);

      sgesdd(&jobz, &N, &M, A, &lda, S, Vt, &ldvt, U, &ldu,
             work.data(), &lwork, iwork.data(), &info);
      return info;
    }

    int mkl_gesdd_d(char jobz, int M, int N, double* A, int lda,
                    double* S, double* U, int ldu, double* Vt, int ldvt) {
      int info;
      int mn = std::min(M, N);
      std::vector<int> iwork(std::max(1, 8 * mn));

      double work_query;
      int lwork = -1;
      dgesdd(&jobz, &N, &M, A, &lda, S, Vt, &ldvt, U, &ldu,
             &work_query, &lwork, iwork.data(), &info);
      if(info != 0) return info;

      lwork = static_cast<int>(work_query);
      std::vector<double> work(lwork);

      dgesdd(&jobz, &N, &M, A, &lda, S, Vt, &ldvt, U, &ldu,
             work.data(), &lwork, iwork.data(), &info);
      return info;
    }

    // ---- SYEV (symmetric eigenvalue) ----

    int mkl_syev_f(char jobz, int N, float* A, int lda, float* W) {
      int info;
      char uplo = 'U';

      float work_query;
      int lwork = -1;
      ssyev(&jobz, &uplo, &N, A, &lda, W, &work_query, &lwork, &info);
      if(info != 0) return info;

      lwork = static_cast<int>(work_query);
      std::vector<float> work(lwork);

      ssyev(&jobz, &uplo, &N, A, &lda, W, work.data(), &lwork, &info);
      return info;
    }

    int mkl_syev_d(char jobz, int N, double* A, int lda, double* W) {
      int info;
      char uplo = 'U';

      double work_query;
      int lwork = -1;
      dsyev(&jobz, &uplo, &N, A, &lda, W, &work_query, &lwork, &info);
      if(info != 0) return info;

      lwork = static_cast<int>(work_query);
      std::vector<double> work(lwork);

      dsyev(&jobz, &uplo, &N, A, &lda, W, work.data(), &lwork, &info);
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

    int mkl_getrf_f(int M, int N, float* A, int lda, int* ipiv) {
      auto AT = lapack_row_to_col(A, M, N, lda);
      int info, m = M, n = N, _lda = M;
      sgetrf(&m, &n, AT.data(), &_lda, ipiv, &info);
      lapack_col_to_row(AT.data(), M, N, A, lda);
      return info;
    }

    int mkl_getrf_d(int M, int N, double* A, int lda, int* ipiv) {
      auto AT = lapack_row_to_col(A, M, N, lda);
      int info, m = M, n = N, _lda = M;
      dgetrf(&m, &n, AT.data(), &_lda, ipiv, &info);
      lapack_col_to_row(AT.data(), M, N, A, lda);
      return info;
    }

    // ---- GETRI (inverse from LU) ----

    int mkl_getri_f(int N, float* A, int lda, const int* ipiv) {
      auto AT = lapack_row_to_col(A, N, N, lda);
      int info, n = N, _lda = N;
      std::vector<int> ipiv_copy(ipiv, ipiv + N);
      float work_query;
      int lwork = -1;
      sgetri(&n, AT.data(), &_lda, ipiv_copy.data(), &work_query, &lwork, &info);
      if(info != 0) return info;
      lwork = static_cast<int>(work_query);
      std::vector<float> work(lwork);
      sgetri(&n, AT.data(), &_lda, ipiv_copy.data(), work.data(), &lwork, &info);
      lapack_col_to_row(AT.data(), N, N, A, lda);
      return info;
    }

    int mkl_getri_d(int N, double* A, int lda, const int* ipiv) {
      auto AT = lapack_row_to_col(A, N, N, lda);
      int info, n = N, _lda = N;
      std::vector<int> ipiv_copy(ipiv, ipiv + N);
      double work_query;
      int lwork = -1;
      dgetri(&n, AT.data(), &_lda, ipiv_copy.data(), &work_query, &lwork, &info);
      if(info != 0) return info;
      lwork = static_cast<int>(work_query);
      std::vector<double> work(lwork);
      dgetri(&n, AT.data(), &_lda, ipiv_copy.data(), work.data(), &lwork, &info);
      lapack_col_to_row(AT.data(), N, N, A, lda);
      return info;
    }

    // ---- GEQRF (QR factorization) ----
    // LAPACK expects column-major; our data is row-major.
    // Unlike SVD/eigenvalue, the dimension-swap trick does NOT work for QR
    // (QR(A^T) ≠ QR(A)), so we transpose explicitly.

    int mkl_geqrf_f(int M, int N, float* A, int lda, float* tau) {
      auto AT = lapack_row_to_col(A, M, N, lda);
      int info, m = M, n = N, _lda = M;
      float work_query;
      int lwork = -1;
      sgeqrf(&m, &n, AT.data(), &_lda, tau, &work_query, &lwork, &info);
      if(info != 0) return info;
      lwork = static_cast<int>(work_query);
      std::vector<float> work(lwork);
      sgeqrf(&m, &n, AT.data(), &_lda, tau, work.data(), &lwork, &info);
      lapack_col_to_row(AT.data(), M, N, A, lda);
      return info;
    }

    int mkl_geqrf_d(int M, int N, double* A, int lda, double* tau) {
      auto AT = lapack_row_to_col(A, M, N, lda);
      int info, m = M, n = N, _lda = M;
      double work_query;
      int lwork = -1;
      dgeqrf(&m, &n, AT.data(), &_lda, tau, &work_query, &lwork, &info);
      if(info != 0) return info;
      lwork = static_cast<int>(work_query);
      std::vector<double> work(lwork);
      dgeqrf(&m, &n, AT.data(), &_lda, tau, work.data(), &lwork, &info);
      lapack_col_to_row(AT.data(), M, N, A, lda);
      return info;
    }

    // ---- ORGQR (form Q from Householder reflectors) ----

    int mkl_orgqr_f(int M, int N, int K, float* A, int lda, const float* tau) {
      auto AT = lapack_row_to_col(A, M, N, lda);
      int info, m = M, n = N, k = K, _lda = M;
      std::vector<float> tau_copy(tau, tau + K);
      float work_query;
      int lwork = -1;
      sorgqr(&m, &n, &k, AT.data(), &_lda, tau_copy.data(), &work_query, &lwork, &info);
      if(info != 0) return info;
      lwork = static_cast<int>(work_query);
      std::vector<float> work(lwork);
      sorgqr(&m, &n, &k, AT.data(), &_lda, tau_copy.data(), work.data(), &lwork, &info);
      lapack_col_to_row(AT.data(), M, N, A, lda);
      return info;
    }

    int mkl_orgqr_d(int M, int N, int K, double* A, int lda, const double* tau) {
      auto AT = lapack_row_to_col(A, M, N, lda);
      int info, m = M, n = N, k = K, _lda = M;
      std::vector<double> tau_copy(tau, tau + K);
      double work_query;
      int lwork = -1;
      dorgqr(&m, &n, &k, AT.data(), &_lda, tau_copy.data(), &work_query, &lwork, &info);
      if(info != 0) return info;
      lwork = static_cast<int>(work_query);
      std::vector<double> work(lwork);
      dorgqr(&m, &n, &k, AT.data(), &_lda, tau_copy.data(), work.data(), &lwork, &info);
      lapack_col_to_row(AT.data(), M, N, A, lda);
      return info;
    }

    // ---- GELSD (least-squares solve via SVD) ----

    int mkl_gelsd_f(int M, int N, int NRHS, float* A, int lda,
                    float* B, int ldb, float* S, float rcond, int* rank) {
      int mx = std::max(M, N);
      auto AT = lapack_row_to_col(A, M, N, lda);
      auto BT = lapack_row_to_col(B, mx, NRHS, ldb);
      int info, m = M, n = N, nrhs = NRHS, _lda = M, _ldb = mx;
      float work_query;
      int lwork = -1, iwork_query;
      sgelsd(&m, &n, &nrhs, AT.data(), &_lda, BT.data(), &_ldb,
             S, &rcond, rank, &work_query, &lwork, &iwork_query, &info);
      if(info != 0) return info;
      lwork = static_cast<int>(work_query);
      std::vector<float> work(lwork);
      std::vector<int> iwork(iwork_query);
      sgelsd(&m, &n, &nrhs, AT.data(), &_lda, BT.data(), &_ldb,
             S, &rcond, rank, work.data(), &lwork, iwork.data(), &info);
      // Copy first N rows from column-major BT (stride mx) back to row-major B.
      // Can't use lapack_col_to_row here: BT stride is mx, not N.
      for(int i = 0; i < N; i++)
        for(int j = 0; j < NRHS; j++)
          B[i * ldb + j] = BT[j * mx + i];
      return info;
    }

    int mkl_gelsd_d(int M, int N, int NRHS, double* A, int lda,
                    double* B, int ldb, double* S, double rcond, int* rank) {
      int mx = std::max(M, N);
      auto AT = lapack_row_to_col(A, M, N, lda);
      auto BT = lapack_row_to_col(B, mx, NRHS, ldb);
      int info, m = M, n = N, nrhs = NRHS, _lda = M, _ldb = mx;
      double work_query;
      int lwork = -1, iwork_query;
      dgelsd(&m, &n, &nrhs, AT.data(), &_lda, BT.data(), &_ldb,
             S, &rcond, rank, &work_query, &lwork, &iwork_query, &info);
      if(info != 0) return info;
      lwork = static_cast<int>(work_query);
      std::vector<double> work(lwork);
      std::vector<int> iwork(iwork_query);
      dgelsd(&m, &n, &nrhs, AT.data(), &_lda, BT.data(), &_ldb,
             S, &rcond, rank, work.data(), &lwork, iwork.data(), &info);
      for(int i = 0; i < N; i++)
        for(int j = 0; j < NRHS; j++)
          B[i * ldb + j] = BT[j * mx + i];
      return info;
    }

  } // anonymous namespace

  static const int _mkl_lapack_reg = []() {
    auto mkl_f = LapackOps<float>::instance().backends(Backend::Mkl);
    mkl_f.add<LapackOps<float>::GesddSig>(LapackOp::gesdd, mkl_gesdd_f, "MKL sgesdd");
    mkl_f.add<LapackOps<float>::SyevSig>(LapackOp::syev, mkl_syev_f, "MKL ssyev");
    mkl_f.add<LapackOps<float>::GetrfSig>(LapackOp::getrf, mkl_getrf_f, "MKL sgetrf");
    mkl_f.add<LapackOps<float>::GetriSig>(LapackOp::getri, mkl_getri_f, "MKL sgetri");
    mkl_f.add<LapackOps<float>::GeqrfSig>(LapackOp::geqrf, mkl_geqrf_f, "MKL sgeqrf");
    mkl_f.add<LapackOps<float>::OrgqrSig>(LapackOp::orgqr, mkl_orgqr_f, "MKL sorgqr");
    mkl_f.add<LapackOps<float>::GelsdSig>(LapackOp::gelsd, mkl_gelsd_f, "MKL sgelsd");

    auto mkl_d = LapackOps<double>::instance().backends(Backend::Mkl);
    mkl_d.add<LapackOps<double>::GesddSig>(LapackOp::gesdd, mkl_gesdd_d, "MKL dgesdd");
    mkl_d.add<LapackOps<double>::SyevSig>(LapackOp::syev, mkl_syev_d, "MKL dsyev");
    mkl_d.add<LapackOps<double>::GetrfSig>(LapackOp::getrf, mkl_getrf_d, "MKL dgetrf");
    mkl_d.add<LapackOps<double>::GetriSig>(LapackOp::getri, mkl_getri_d, "MKL dgetri");
    mkl_d.add<LapackOps<double>::GeqrfSig>(LapackOp::geqrf, mkl_geqrf_d, "MKL dgeqrf");
    mkl_d.add<LapackOps<double>::OrgqrSig>(LapackOp::orgqr, mkl_orgqr_d, "MKL dorgqr");
    mkl_d.add<LapackOps<double>::GelsdSig>(LapackOp::gelsd, mkl_gelsd_d, "MKL dgelsd");

    return 0;
  }();

  } // namespace icl::math
