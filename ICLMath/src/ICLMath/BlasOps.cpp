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
        case BlasOp::vadd:  return "vadd";
        case BlasOp::vsub:  return "vsub";
        case BlasOp::vmul:  return "vmul";
        case BlasOp::vdiv:  return "vdiv";
        case BlasOp::vsadd: return "vsadd";
        case BlasOp::vsmul: return "vsmul";
      }
      return "?";
    }

    template<class T>
    BlasOps<T>::BlasOps() {
      addSelector<GemmSig>(BlasOp::gemm);
      addSelector<VecBinarySig>(BlasOp::vadd);
      addSelector<VecBinarySig>(BlasOp::vsub);
      addSelector<VecBinarySig>(BlasOp::vmul);
      addSelector<VecBinarySig>(BlasOp::vdiv);
      addSelector<VecScalarSig>(BlasOp::vsadd);
      addSelector<VecScalarSig>(BlasOp::vsmul);
    }

    template<class T>
    BlasOps<T>& BlasOps<T>::instance() {
      static BlasOps<T> ops;
      return ops;
    }

    // Cached dispatch methods — resolve on first call via function-local static
#define BLAS_CACHED_BINARY(NAME, KEY)                                        \
    template<class T>                                                        \
    void BlasOps<T>::NAME(const T* a, const T* b, T* dst, int n) {           \
      static auto* impl = instance()                                         \
          .template getSelector<VecBinarySig>(BlasOp::KEY).resolveOrThrow(); \
      impl->apply(a, b, dst, n);                                             \
    }
#define BLAS_CACHED_SCALAR(NAME, KEY)                                        \
    template<class T>                                                        \
    void BlasOps<T>::NAME(const T* src, T scalar, T* dst, int n) {           \
      static auto* impl = instance()                                         \
          .template getSelector<VecScalarSig>(BlasOp::KEY).resolveOrThrow(); \
      impl->apply(src, scalar, dst, n);                                      \
    }

    BLAS_CACHED_BINARY(vadd, vadd)
    BLAS_CACHED_BINARY(vsub, vsub)
    BLAS_CACHED_BINARY(vmul, vmul)
    BLAS_CACHED_BINARY(vdiv, vdiv)
    BLAS_CACHED_SCALAR(vsadd, vsadd)
    BLAS_CACHED_SCALAR(vsmul, vsmul)

#undef BLAS_CACHED_BINARY
#undef BLAS_CACHED_SCALAR

    template struct BlasOps<float>;
    template struct BlasOps<double>;

  } // namespace math
} // namespace icl
