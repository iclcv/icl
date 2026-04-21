// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/math/BlasOps.h>

using namespace icl::utils;

namespace icl::math {
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

  // Cached dispatch methods are now inline in BlasOps.h

  template struct BlasOps<float>;
  template struct BlasOps<double>;

  } // namespace icl::math