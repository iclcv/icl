// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/math/FFTOps.h>

namespace icl::math {
  const char* toString(FFTOp op) {
    switch(op) {
      case FFTOp::r2c:     return "r2c";
      case FFTOp::c2c:     return "c2c";
      case FFTOp::inv_c2c: return "inv_c2c";
    }
    return "?";
  }

  template<class T>
  FFTOps<T>::FFTOps() {
    addSelector<R2CSig>(FFTOp::r2c);
    addSelector<C2CSig>(FFTOp::c2c);
    addSelector<InvC2CSig>(FFTOp::inv_c2c);
  }

  template<class T>
  FFTOps<T>& FFTOps<T>::instance() {
    static FFTOps<T> ops;
    return ops;
  }

  template struct FFTOps<float>;
  template struct FFTOps<double>;

  } // namespace icl::math