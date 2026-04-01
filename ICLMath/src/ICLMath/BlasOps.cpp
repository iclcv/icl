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
      }
      return "?";
    }

    template<class T>
    BlasOps<T>::BlasOps() {
      addSelector<GemmSig>(BlasOp::gemm);
    }

    template<class T>
    BlasOps<T>& BlasOps<T>::instance() {
      static BlasOps<T> ops;
      return ops;
    }

    template struct BlasOps<float>;
    template struct BlasOps<double>;

  } // namespace math
} // namespace icl
