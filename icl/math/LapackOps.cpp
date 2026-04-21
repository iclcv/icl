// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/math/LapackOps.h>

using namespace icl::utils;

namespace icl::math {

  // Row-major ↔ column-major transpose helpers for LAPACK backends.
  // LAPACK expects column-major; ICL's DynMatrix is row-major.
  template<class T>
  void lapack_row_to_col(const T* A, int M, int N, int lda, T* AT) {
    for(int i = 0; i < M; i++)
      for(int j = 0; j < N; j++)
        AT[j * M + i] = A[i * lda + j];
  }

  template<class T>
  std::vector<T> lapack_row_to_col(const T* A, int M, int N, int lda) {
    std::vector<T> AT(M * N);
    lapack_row_to_col(A, M, N, lda, AT.data());
    return AT;
  }

  template<class T>
  void lapack_col_to_row(const T* AT, int M, int N, T* A, int lda) {
    for(int i = 0; i < M; i++)
      for(int j = 0; j < N; j++)
        A[i * lda + j] = AT[j * M + i];
  }

  template void lapack_row_to_col(const float*, int, int, int, float*);
  template void lapack_row_to_col(const double*, int, int, int, double*);
  template std::vector<float> lapack_row_to_col(const float*, int, int, int);
  template std::vector<double> lapack_row_to_col(const double*, int, int, int);
  template void lapack_col_to_row(const float*, int, int, float*, int);
  template void lapack_col_to_row(const double*, int, int, double*, int);
  const char* toString(LapackOp op) {
    switch(op) {
      case LapackOp::gesdd: return "gesdd";
      case LapackOp::syev:  return "syev";
      case LapackOp::getrf: return "getrf";
      case LapackOp::getri: return "getri";
      case LapackOp::geqrf: return "geqrf";
      case LapackOp::orgqr: return "orgqr";
      case LapackOp::gelsd: return "gelsd";
    }
    return "?";
  }

  template<class T>
  LapackOps<T>::LapackOps() {
    addSelector<GesddSig>(LapackOp::gesdd);
    addSelector<SyevSig>(LapackOp::syev);
    addSelector<GetrfSig>(LapackOp::getrf);
    addSelector<GetriSig>(LapackOp::getri);
    addSelector<GeqrfSig>(LapackOp::geqrf);
    addSelector<OrgqrSig>(LapackOp::orgqr);
    addSelector<GelsdSig>(LapackOp::gelsd);
  }

  template<class T>
  LapackOps<T>& LapackOps<T>::instance() {
    static LapackOps<T> ops;
    return ops;
  }

  template struct LapackOps<float>;
  template struct LapackOps<double>;

  } // namespace icl::math