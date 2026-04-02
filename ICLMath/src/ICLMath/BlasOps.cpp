// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLMath/BlasOps.h>

using namespace icl::utils;

namespace icl {
  namespace math {

    const char* toString(BlasOp op) {
      switch(op) {
        case BlasOp::gemm:  return "gemm";
        case BlasOp::gemv:  return "gemv";
        case BlasOp::vadd:  return "vadd";
        case BlasOp::vsub:  return "vsub";
        case BlasOp::vmul:  return "vmul";
        case BlasOp::vdiv:  return "vdiv";
        case BlasOp::vsadd: return "vsadd";
        case BlasOp::vsmul: return "vsmul";
        case BlasOp::dot:   return "dot";
        case BlasOp::nrm2:  return "nrm2";
        case BlasOp::asum:  return "asum";
        case BlasOp::axpy:  return "axpy";
        case BlasOp::scal:  return "scal";
      }
      return "?";
    }

    template<class T>
    BlasOps<T>::BlasOps() {
      // Level 3
      addSelector<GemmSig>(BlasOp::gemm);
      // Level 2
      addSelector<GemvSig>(BlasOp::gemv);
      // Level 1 binary/scalar
      addSelector<VecBinarySig>(BlasOp::vadd);
      addSelector<VecBinarySig>(BlasOp::vsub);
      addSelector<VecBinarySig>(BlasOp::vmul);
      addSelector<VecBinarySig>(BlasOp::vdiv);
      addSelector<VecScalarSig>(BlasOp::vsadd);
      addSelector<VecScalarSig>(BlasOp::vsmul);
      // Level 1 reductions/in-place
      addSelector<DotSig>(BlasOp::dot);
      addSelector<NrmSig>(BlasOp::nrm2);
      addSelector<NrmSig>(BlasOp::asum);
      addSelector<AxpySig>(BlasOp::axpy);
      addSelector<ScalSig>(BlasOp::scal);
    }

    template<class T>
    BlasOps<T>& BlasOps<T>::instance() {
      static BlasOps<T> ops;
      return ops;
    }

    // Cached dispatch methods — resolve on first call via function-local static

#define BLAS_CACHED_BINARY(NAME, KEY) \
    template<class T> \
    void BlasOps<T>::NAME(const T* a, const T* b, T* dst, int n) { \
      static auto* impl = instance().template getSelector<VecBinarySig>(BlasOp::KEY).resolveOrThrow(); \
      impl->apply(a, b, dst, n); \
    }
#define BLAS_CACHED_SCALAR(NAME, KEY) \
    template<class T> \
    void BlasOps<T>::NAME(const T* src, T scalar, T* dst, int n) { \
      static auto* impl = instance().template getSelector<VecScalarSig>(BlasOp::KEY).resolveOrThrow(); \
      impl->apply(src, scalar, dst, n); \
    }

    BLAS_CACHED_BINARY(vadd, vadd)
    BLAS_CACHED_BINARY(vsub, vsub)
    BLAS_CACHED_BINARY(vmul, vmul)
    BLAS_CACHED_BINARY(vdiv, vdiv)
    BLAS_CACHED_SCALAR(vsadd, vsadd)
    BLAS_CACHED_SCALAR(vsmul, vsmul)

#undef BLAS_CACHED_BINARY
#undef BLAS_CACHED_SCALAR

    template<class T>
    T BlasOps<T>::dot(const T* a, const T* b, int n) {
      static auto* impl = instance().template getSelector<DotSig>(BlasOp::dot).resolveOrThrow();
      return impl->apply(a, b, n);
    }

    template<class T>
    T BlasOps<T>::nrm2(const T* x, int n) {
      static auto* impl = instance().template getSelector<NrmSig>(BlasOp::nrm2).resolveOrThrow();
      return impl->apply(x, n);
    }

    template<class T>
    T BlasOps<T>::asum(const T* x, int n) {
      static auto* impl = instance().template getSelector<NrmSig>(BlasOp::asum).resolveOrThrow();
      return impl->apply(x, n);
    }

    template<class T>
    void BlasOps<T>::axpy(T alpha, const T* x, T* y, int n) {
      static auto* impl = instance().template getSelector<AxpySig>(BlasOp::axpy).resolveOrThrow();
      impl->apply(alpha, x, y, n);
    }

    template<class T>
    void BlasOps<T>::scal(T alpha, T* x, int n) {
      static auto* impl = instance().template getSelector<ScalSig>(BlasOp::scal).resolveOrThrow();
      impl->apply(alpha, x, n);
    }

    template<class T>
    void BlasOps<T>::gemv(bool trans, int M, int N, T alpha,
                           const T* A, int lda, const T* x,
                           T beta, T* y) {
      static auto* impl = instance().template getSelector<GemvSig>(BlasOp::gemv).resolveOrThrow();
      impl->apply(trans, M, N, alpha, A, lda, x, beta, y);
    }

    template struct BlasOps<float>;
    template struct BlasOps<double>;

  } // namespace math
} // namespace icl
