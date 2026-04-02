// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLMath/LapackOps.h>

using namespace icl::utils;

namespace icl::math {
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