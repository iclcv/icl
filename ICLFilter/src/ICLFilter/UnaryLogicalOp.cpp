// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLFilter/UnaryLogicalOp.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl {
  namespace filter{

    const char* toString(UnaryLogicalOp::Op op) {
      switch(op) {
        case UnaryLogicalOp::Op::withVal: return "withVal";
        case UnaryLogicalOp::Op::noVal: return "noVal";
      }
      return "?";
    }

    core::ImageBackendDispatching& UnaryLogicalOp::prototype() {
      static core::ImageBackendDispatching proto;
      static bool init = [&] {
        proto.addSelector<WithValSig>(Op::withVal);
        proto.addSelector<NoValSig>(Op::noVal);
        return true;
      }();
      (void)init;
      return proto;
    }

    // Constructor — clones selectors from the class prototype
    UnaryLogicalOp::UnaryLogicalOp(optype t, icl32s val)
      : ImageBackendDispatching(prototype()),
        m_eOpType(t), m_dValue(val)
    {}

    void UnaryLogicalOp::apply(const Image &src, Image &dst) {
      ICLASSERT_RETURN(src.getDepth() == depth8u || src.getDepth() == depth16s || src.getDepth() == depth32s);
      if(!prepare(dst, src)) return;

      if(m_eOpType == notOp) {
        getSelector<NoValSig>(Op::noVal).resolve(src)->apply(src, dst);
      } else {
        getSelector<WithValSig>(Op::withVal).resolve(src)->apply(src, dst, m_dValue, (int)m_eOpType);
      }
    }

  } // namespace filter
}
